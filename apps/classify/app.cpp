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
#include <nnets/classifier.h>
#include <nnets/conv2d.h>
#include <nnets/dense.h>
#include <nnets/image_sampler.h>
#include <nnets/pooling2d.h>
#include <nnets/softmax.h>
#include <utils/profiler.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <thread>
#include <regex>


/**
    A dog breed classifier.
    An example of Classifier implementation recognizing 119 dogs breeds and 1 cat breed.
    Contains a ResNeXt-like convolution neural net constructed programmatically. Requires a chunkfile with the model data.
*/
class DogClassifier : public Beatmup::NNets::Classifier {
private:
    /**
        Add connections between operations in a unit (block).
        \param after            The preceding operation name
        \param prefix           The unit prefix
        \param shuffleInput     If `true`, a shuffling operation is applied at input
    */
    inline void addUnit(std::string& after, const std::string& prefix, bool shuffleInput = false) {
        // main connections
        addConnection(after, prefix + "-pw", 0, 0, shuffleInput ? 8 : 0);
        addConnection(prefix + "-pw", prefix + "-dw", 0, 0);

        // residual connection
        addConnection(after, prefix + "-dw", 0, 1);
        after = prefix + "-dw";
    }

    /**
        Adds a connection to a specific operation
        \param after            A preceding operation name
        \param name             An operation to connect
    */
    inline void addLayer(std::string& after, const std::string& name) {
        addConnection(after, name);
        after = name;
    }

    Beatmup::ChunkFile modelData;       //!< model data (convolution filters, biases, dense matrix)

public:
    /**
        Creates a DogClassifier instance.
        \param context              A context instance the classifier will use to store its internal data
        \param pathToModelData      Relative path to the chunkfile containing the model data.
    */
    inline DogClassifier(Beatmup::Context& context, const char* pathToModelData):
        modelData(pathToModelData),
        Classifier(context, modelData)
    {
        using namespace Beatmup::NNets;

        // Create and add operations in the order they are executed. This does not add connections between the ops though.
        append({
            new ImageSampler("input", Beatmup::IntPoint(385, 385)),
            new Conv2D("b0-conv", 5, 3, 32, 2, Size::Padding::VALID, true, 1, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b1-stage1-pw", 1, 32, 32, 1, Size::Padding::SAME, true, 1, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b1-stage1-dw", 3, 32, 32, 1, Size::Padding::SAME, true, 8, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b1-stage2-pw", 1, 32, 32, 1, Size::Padding::SAME, true, 1, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b1-stage2-dw", 3, 32, 32, 1, Size::Padding::SAME, true, 8, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b1-scale", 3, 32, 64, 2, Size::Padding::VALID, true, 8, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-stage1-pw", 1, 64, 64, 1, Size::Padding::SAME, true, 2, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-stage1-dw", 3, 64, 64, 1, Size::Padding::SAME, true, 16, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-stage2-pw", 1, 64, 64, 1, Size::Padding::SAME, true, 2, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-stage2-dw", 3, 64, 64, 1, Size::Padding::SAME, true, 16, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-stage3-pw", 1, 64, 64, 1, Size::Padding::SAME, true, 2, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-stage3-dw", 3, 64, 64, 1, Size::Padding::SAME, true, 16, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b2-scale", 3, 64, 96, 2, Size::Padding::VALID, true, 8, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-stage1-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-stage1-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-stage2-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-stage2-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-stage3-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-stage3-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b3-scale", 3, 96, 96, 2, Size::Padding::VALID, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage1-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage1-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage2-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage2-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage3-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage3-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage4-pw", 1, 96, 96, 1, Size::Padding::SAME, true, 3, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-stage4-dw", 3, 96, 96, 1, Size::Padding::SAME, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b4-scale", 3, 96, 192, 2, Size::Padding::VALID, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage1-pw", 1, 192, 192, 1, Size::Padding::SAME, true, 6, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage1-dw", 3, 192, 192, 1, Size::Padding::SAME, true, 48, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage2-pw", 1, 192, 192, 1, Size::Padding::SAME, true, 6, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage2-dw", 3, 192, 192, 1, Size::Padding::SAME, true, 48, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage3-pw", 1, 192, 192, 1, Size::Padding::SAME, true, 6, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage3-dw", 3, 192, 192, 1, Size::Padding::SAME, true, 48, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage4-pw", 1, 192, 192, 1, Size::Padding::SAME, true, 6, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-stage4-dw", 3, 192, 192, 1, Size::Padding::SAME, true, 48, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b5-scale", 3, 192, 192, 2, Size::Padding::VALID, true, 48, ActivationFunction::SIGMOID_LIKE),
            new Conv2D("b6-conv1", 3, 192, 192, 1, Size::Padding::VALID, true, 24, ActivationFunction::SIGMOID_LIKE),
            new Pooling2D("b7-pool", Pooling2D::Operator::AVERAGE, 3, 1, Size::Padding::VALID),
            new Dense(context, "Dense", 120, true),
            new Softmax("Softmax")
        });

        // Add connections between operations
        std::string ptr = getFirstOperation().getName();
        addLayer(ptr, "b0-conv");
        addUnit (ptr, "b1-stage1");
        addUnit (ptr, "b1-stage2");
        addLayer(ptr, "b1-scale");
        addUnit (ptr, "b2-stage1", true);
        addUnit (ptr, "b2-stage2", true);
        addUnit (ptr, "b2-stage3", true);
        addLayer(ptr, "b2-scale");
        addUnit (ptr, "b3-stage1", true);
        addUnit (ptr, "b3-stage2", true);
        addUnit (ptr, "b3-stage3", true);
        addLayer(ptr, "b3-scale");
        addUnit (ptr, "b4-stage1", true);
        addUnit (ptr, "b4-stage2", true);
        addUnit (ptr, "b4-stage3", true);
        addUnit (ptr, "b4-stage4", true);
        addLayer(ptr, "b4-scale");
        addUnit (ptr, "b5-stage1", true);
        addUnit (ptr, "b5-stage2", true);
        addUnit (ptr, "b5-stage3", true);
        addUnit (ptr, "b5-stage4", true);
        addLayer(ptr, "b5-scale");
        addLayer(ptr, "b6-conv1");
        addLayer(ptr, "b7-pool");
        addLayer(ptr, "Dense");
        addLayer(ptr, "Softmax");
    }
};


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
    The chunkfile actually contains a serialized model, so there is a simpler way to build the classifier using Beatmup::NNets::DeserializedModel
    (see the Java sample). However, for illustration purposes, in this app the model is constructed by hand layer-by-layer.
    The path is given relatively to the repository root folder.
*/
static constexpr const char* PATH_TO_MODEL_DATA = "android/app/src/main/assets/dog_classifier.chunks";

int mainThatThrowsExceptions(int argc, char* argv[]) {
    // Make sure the model data is accessible
    if (!Beatmup::ChunkFile::readable(PATH_TO_MODEL_DATA))
        throw std::runtime_error("Cannot reach the model data file: " + std::string(PATH_TO_MODEL_DATA));

    // Create a context
    Beatmup::Context ctx;

    // Init classifier
    DogClassifier classifier(ctx, PATH_TO_MODEL_DATA);
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
    classifier.start(dummyInput);
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
            classifier(input);
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
        for (auto p : classifier.getProbabilities())
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
            classifier(input);
            auto finishTime = std::chrono::high_resolution_clock::now();
            const float time = std::chrono::duration<float, std::milli>(finishTime - startTime).count();
            averageTime.update(time);
            fastestRunTime = numImages > 0 ? std::min(fastestRunTime, time) : time;

            // Check if the prediction matches the class label, if given
            const auto& probs = classifier.getProbabilities();
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