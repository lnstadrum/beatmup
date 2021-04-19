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
    Upsamples an image using a convolutional neural net inferred on GPU.
    A basic example of using Resampler to infer the net is much simpler; in this app there is a lot of code
    to be able to meter only the inference (warming up GPU, explicitly transferring pixels, etc.)
*/
#include <bitmap/internal_bitmap.h>
#include <bitmap/resampler.h>
#include <gpu/swapper.h>
#include <iostream>
#include <algorithm>
#include <chrono>


int main(int argc, char* argv[]) {
    enum Args {
        INPUT_FILENAME = 1,
        OUTPUT_FILENAME,
        REPEAT_COUNT
    };

    if (argc <= Args::INPUT_FILENAME) {
        std::cout
            << "Usage: resample <input file name> [<output file name>] [<repeat count>]" << std::endl
            << "BMP files are supported only." << std::endl
            << std::endl;
        return 1;
    }

    // Create a context
    Beatmup::Context ctx;
    ctx.limitWorkerCount(1);

    // Load input
    Beatmup::InternalBitmap input(ctx, argv[INPUT_FILENAME]);

    // Initialize the output bitmap
    // When running GLES 2.0, QuadByte is the optimal format to transfer from GPU memory.
    Beatmup::InternalBitmap output(
        ctx,
        Beatmup::PixelFormat::QuadByte,
        2 * input.getWidth(),
        2 * input.getHeight()
    );

    // Instantiate a resampler
    Beatmup::BitmapResampler resampler(ctx);
    resampler.setMode(Beatmup::BitmapResampler::Mode::CONVNET);

    // Warm up GPU
    std::cout << "Preparing GPU... " << std::endl;
    std::string vendor, renderer;
    ctx.queryGpuInfo(vendor, renderer);
    std::cout << vendor << " | " << renderer << std::endl;

    // Run the task on a dummy bitmap first to warm up GPU, compile shaders and allocate output
    std::cout << "Compiling shaders... ";
    float time;
    Beatmup::InternalBitmap dummyInput(ctx, input.getPixelFormat(), input.getWidth(), input.getHeight());
    resampler.setInput(&dummyInput);
    resampler.setOutput(&output);
    time = ctx.performTask(resampler);
    std::cout << time << " ms" << std::endl;

    // Transfer input pixels to GPU. This is not necessary: Resampler (as well as pretty much any other task) does it itself if needed.
    // It is done here to properly meter the inference time (not to mix in the pixel transfer time).
    std::cout << "Transfering input to GPU... " << std::endl;
    Beatmup::Swapper::pushPixels(input);

    // Run the inference (possibly multiple times)
    std::cout << "Processing... " << std::endl;
    resampler.setInput(&input);
    resampler.setOutput(&output);
    const int repeat = argc > Args::REPEAT_COUNT ? std::stoi(argv[Args::REPEAT_COUNT]) : 1;
    float timeSum = 0, sqrTimeSum = 0, minTime, maxTime;
    for (int i = 0; i < repeat; ++i) {
        time = ctx.performTask(resampler);
        std::cout << "  " << time << " ms" << std::endl;;
        timeSum += time;
        sqrTimeSum += time * time;
        if (i == 0)
            minTime = maxTime = time;
        else {
            minTime = std::min(minTime, time);
            maxTime = std::max(maxTime, time);
        }
    }

    // Write out time stats if asked
    if (repeat > 1) {
        const float std = sqrtf(sqrTimeSum / repeat - (timeSum * timeSum) / (repeat * repeat));
        std::cout
            << "Average: " << timeSum / repeat << " ms, min: " << minTime
            << " ms, max: " << maxTime << " ms, std: " << std << " ms" << std::endl;
    }

    // Save output to a file
    const std::string inputFilename = argv[Args::INPUT_FILENAME];
    const std::string outputFilename = argc > Args::OUTPUT_FILENAME ?
        argv[Args::OUTPUT_FILENAME] :
        inputFilename.substr(0, inputFilename.length() - 4) + "_x2" + inputFilename.substr(inputFilename.length() - 4);
    std::cout << "Saving result to " << outputFilename << std::endl;
    output.saveBmp(outputFilename.c_str());

    return 0;
}
