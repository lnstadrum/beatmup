/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
    Classifier test app
*/

//#define PROFILE
    //!< When defined, a Profiler is attached to the Model in order to meter every operation execution time. This makes the processing slower.

#include <bitmap/internal_bitmap.h>
#include <nnets/deserialized_model.h>
#include <nnets/inference_task.h>
#include <utils/profiler.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <regex>


/**
    Blocks until a given ProgressTracking finishes, printing the progress bar in the meantime.
*/
void waitTillDone(const Beatmup::ProgressTracking& progress, Beatmup::Context& ctx) {
    do {
        ctx.check();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(250ms);
        progress.printOutProgressBar();
    } while (!progress.done());
    std::cout << std::endl;
}


/**
    Command line arguments
*/
enum Args {
    INPUT_FILENAME = 1,
    OUTPUT_FILENAME = 2
};


/**
    Relative path to the chunkfile containing the model data.
    This app uses the same chunkfile as in the Android dog classification test sample.
    The path is given relatively to the repository root folder.
*/
static constexpr const char* PATH_TO_MODEL_DATA = "android/app/src/main/assets/dog_classifier.chunks";

int mainThatThrowsExceptions(int argc, char* argv[]) {
    // Make sure the model data is accessible
    if (!Beatmup::ChunkFile::readable(PATH_TO_MODEL_DATA))
        throw std::runtime_error("Cannot reach the model data file: " + std::string(PATH_TO_MODEL_DATA));

    // Create a context
    Beatmup::Context ctx;

    // Init classifier. By convention, the chunk having empty name contains the model description in a YML-like format
    // that can be feed to DeserializedModel constructor.
    Beatmup::ChunkFile modelData(PATH_TO_MODEL_DATA);
    Beatmup::NNets::DeserializedModel classifier(ctx, modelData.read<std::string>(""));

    // Keep a reference to the Softmax layer providing the output.
    const auto& softmax = static_cast<Beatmup::NNets::Softmax&>(classifier.getLastOperation());

    // Init the inference task
    Beatmup::NNets::InferenceTask inference(classifier, modelData);

#ifdef PROFILE
    Beatmup::Profiler profiler;
    classifier.setProfiler(&profiler);
#endif

    // Warm up GPU
    std::cout << "Preparing GPU... " << std::endl;
    std::string vendor, renderer;
    ctx.queryGpuInfo(vendor, renderer);
    std::cout << vendor << " | " << renderer << std::endl;

    // Initiate the first run and wait till the model is ready
    Beatmup::InternalBitmap dummyInput(ctx, Beatmup::PixelFormat::TripleByte, 385, 385);
    inference.connect(dummyInput, classifier.getFirstOperation(), 0);
    ctx.submitTask(inference);
    std::cout << "Preparing classifier:" << std::endl;
    waitTillDone(classifier.getPreparingProgress(), ctx);

    // Once the model is ready, we can query its complexity
    const unsigned long madds = classifier.countMultiplyAdds();
    std::cout << "Number of multiply-adds: " << std::setprecision(1) << std::fixed << 1e-6 * madds << "M" << std::endl;
    std::cout << "Used texture memory: " << classifier.getMemorySize() / 1024 << " kb" << std::endl;

    // The first run includes the preparation pass and the inference pass. Wait till the first inference pass is over.
    std::cout << "First pass:" << std::endl;
    waitTillDone(classifier.getInferenceProgress(), ctx);
    ctx.wait();
    ctx.check();

    // Ready to go.
    std::cout << "Inference:" << std::endl;
    const std::string inputFilename(argv[INPUT_FILENAME]);
    float fastestRunTime;

    // Single image input
    if (inputFilename.substr(inputFilename.length() - 4) == ".bmp") {
        Beatmup::InternalBitmap input(ctx, inputFilename.c_str());

#ifdef PROFILE
        profiler.reset();
#endif

        // For debugging and profiling, the inference may be run several times
        static const int numRepeats = 10;
        for (int i = 0; i < numRepeats; ++i) {
            auto startTime = std::chrono::high_resolution_clock::now();
            inference.connect(input, classifier.getFirstOperation());
            ctx.performTask(inference);
            auto finishTime = std::chrono::high_resolution_clock::now();
            const float time = std::chrono::duration<float, std::milli>(finishTime - startTime).count();
            fastestRunTime = i > 0 ? std::min(fastestRunTime, time) : time;
            std::cout << "  " << time << " ms" << std::endl;
        }

        // Write out the predicted probabilities
        std::ostream* ostr = &std::cout;
        if (argc > Args::OUTPUT_FILENAME)
            ostr = new std::ofstream(argv[OUTPUT_FILENAME], std::ios::out);
        else
            std::cout << "Class probabilities:" << std::endl;

        *ostr << std::setprecision(5) << std::fixed;
        for (auto p : softmax.getProbabilities())
            *ostr << p << ", ";
        *ostr << std::endl;

        if (argc > Args::OUTPUT_FILENAME)
            delete ostr;
    }

    // Text listing input
    else {
        // Open the listing, check it is readable
        std::string listingFname(argv[INPUT_FILENAME]);
        std::ifstream listing(listingFname, std::ios::in);
        if (!listing.good()) {
            std::cerr << "Cannot open " << argv[INPUT_FILENAME] << " for reading." << std::endl;
            exit(-2);
        }

        // Get the listing filename path
        auto delimPos = listingFname.find_last_of("/\\");
        const std::string imagesFolder = delimPos == std::string::npos ? "" : listingFname.substr(0, delimPos + 1);

        // Open output report
        std::ofstream* outputReport = nullptr;
        if (argc > Args::OUTPUT_FILENAME)
            outputReport = new std::ofstream(argv[OUTPUT_FILENAME], std::ios::out);

        // Read line by line
        bool meterAccuracy = true;
        Beatmup::MovingAverage<5> averageTime;
        int numImages = 0;
        int score = 0;
        for (std::string line; std::getline(listing, line) && line != ""; ++numImages) {
            // Get image filename and class label, if any
            std::string imageFilename;
            int classLabel;
            const std::regex regex("(.+[^\\s])\\s+(\\d+)$");
            std::smatch match;
            if (std::regex_match(line, match, regex) && match.size() == 3) {
                imageFilename = match[1].str();
                classLabel = std::stoi(match[2].str());
            }
            else {
                imageFilename = line;
                int classLabel = -1;
                meterAccuracy = false;
                if (numImages > 0)
                    std::cout << std::endl << "Cannot get class label for " << imageFilename << std::endl;
            }

            // Read input
            Beatmup::InternalBitmap input(ctx, (imagesFolder + imageFilename).c_str());

            // Run inference
            auto startTime = std::chrono::high_resolution_clock::now();
            inference.connect(input, classifier.getFirstOperation());
            ctx.performTask(inference);
            auto finishTime = std::chrono::high_resolution_clock::now();
            const float time = std::chrono::duration<float, std::milli>(finishTime - startTime).count();
            averageTime.update(time);
            fastestRunTime = numImages > 0 ? std::min(fastestRunTime, time) : time;

            // Check if the prediction matches the class label, if given
            const auto& probs = softmax.getProbabilities();
            if (meterAccuracy) {
                int max = 0;
                for (size_t i = 1; i < probs.size(); ++i)
                    if (probs[i] > probs[max])
                        max = i;
                if (max == classLabel)
                    score++;
            }

            // Write out the predicted probabilities to the output report
            if (outputReport) {
                *outputReport << probs[0];
                for (size_t i = 1; i < probs.size(); ++i)
                    *outputReport << " " << probs[i];
                *outputReport << std::endl;
            }

            // Print out things
            std::cout << '\r' << "[" << std::setw(5) << numImages << "] ";
            std::cout << std::setprecision(2) << std::setw(8) << 1000 / averageTime() << " FPS ";
            if (meterAccuracy)
                std::cout << std::setprecision(3) << std::setw(8) << 100.0f * score / (numImages + 1) << "%  ";
            static const int MAX_LEN = 50;
            std::cout << std::setw(MAX_LEN) << imageFilename.substr(std::max<int>(0, imageFilename.length() - MAX_LEN)).c_str();
            fflush(stdout);

            // Clean up unused GPU ressources
            ctx.getGpuRecycleBin()->emptyBin();
        }

        std::cout << std::endl;
        delete outputReport;
    }

#ifdef PROFILE
    // Print out profiling results
    profiler.report(std::cout);
#endif

    std::cout << "Fastest run: " << std::setprecision(1) << fastestRunTime << " ms, " << 1e-3 * madds / fastestRunTime << " Mmadds / s" << std::endl;

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc <= Args::INPUT_FILENAME) {
        std::cout
            << "Usage: Classify <input file> [<output report>]" << std::endl << std::endl
            << "The input file may be one of the following:" << std::endl
            << " * a BMP file containing an image to classify (the only supported extension is '.bmp', case sensitive);" << std::endl
            << " * a text file listing paths to input images in BMP format," << std::endl
            << " * a text file listing paths to input images in BMP format and corresponding zero-based numbers of correct classes." << std::endl << std::endl
            << "In the text listings, one entry is searched per line, with a single space separing the file path and (optionally) the class label. "
            << "The image paths are treated as relative to the listing file path." << std::endl
            << "If the class labels are provided, the top-1 accuracy is metered and printed during the inference." << std::endl
            << "If the output report path is given, it is filled with class probabilities for every input image." << std::endl
            << std::endl;
        return 1;
    }

    try {
        return mainThatThrowsExceptions(argc, argv);
    }
    catch (std::exception & ex) {
        std::cerr << ex.what() << std::endl;
        return -1;
    }
}