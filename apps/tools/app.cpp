/**
    Resample a bitmap
*/
#include <bitmap/internal_bitmap.h>
#include <bitmap/resampler.h>
#include <bitmap/metric.h>
#include <bitmap/tools.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <string.h>


bool strToPixelFormat(const char* str, Beatmup::PixelFormat& format) {
    if (strcmp(str, "bin") == 0) {
        format = Beatmup::PixelFormat::BinaryMask;
        return true;
    }
    if (strcmp(str, "quad") == 0) {
        format = Beatmup::PixelFormat::QuaternaryMask;
        return true;
    }
    if (strcmp(str, "hex") == 0) {
        format = Beatmup::PixelFormat::HexMask;
        return true;
    }
    if (strcmp(str, "lum") == 0) {
        format = Beatmup::PixelFormat::SingleByte;
        return true;
    }
    if (strcmp(str, "rgb") == 0) {
        format = Beatmup::PixelFormat::TripleByte;
        return true;
    }
    if (strcmp(str, "rgba") == 0) {
        format = Beatmup::PixelFormat::QuadByte;
        return true;
    }
    if (strcmp(str, "LUM") == 0) {
        format = Beatmup::PixelFormat::SingleFloat;
        return true;
    }
    if (strcmp(str, "RGB") == 0) {
        format = Beatmup::PixelFormat::TripleFloat;
        return true;
    }
    if (strcmp(str, "RGBA") == 0) {
        format = Beatmup::PixelFormat::QuadFloat;
        return true;
    }
    return false;
}


int measure(int argc, const char* argv[]) {
    enum Args {
        METRIC = 1,
        INPUT1_FILENAME,
        INPUT2_FILENAME
    };

    if (argc <= Args::INPUT2_FILENAME) {
        std::cerr << "Usage:" << std::endl
                << "  tools measure psnr|l1 <input1.bmp> <input2.bmp>" << std::endl;
        return 1;
    }

    // Create a context
    Beatmup::Context ctx;

    // Load input
    Beatmup::InternalBitmap input1(ctx, argv[INPUT1_FILENAME]);
    Beatmup::InternalBitmap input2(ctx, argv[INPUT2_FILENAME]);

    if (strcmp(argv[Args::METRIC], "psnr") == 0) {
        std::cout << std::setprecision(15) << Beatmup::Metric::psnr(input1, input2) << " dB" << std::endl;
    }
    else {
        // Instantiate a metric
        Beatmup::Metric metric;
        metric.setBitmaps(&input1, &input2);

        // set norm
        if (strcmp(argv[Args::METRIC], "l1") == 0)
            metric.setNorm(Beatmup::Metric::Norm::L1);
        else {
            std::cerr << "Unknown norm: " <<  argv[Args::METRIC] << std::endl;
            return 1;
        }

        // compute
        ctx.performTask(metric);

        // output result
        std::cout << std::setprecision(10) << metric.getResult() << std::endl;
    }

    return 0;
}


int noise(int argc, const char* argv[]) {
    enum Args {
        OUTPUT_WIDTH = 1,
        OUTPUT_HEIGHT,
        OUTPUT_FORMAT,
        OUTPUT_FILENAME
    };

    if (argc <= Args::OUTPUT_FILENAME) {
        std::cerr << "Usage:" << std::endl
                << "    tools noise <width> <height> <format> <output.bmp>" << std::endl;
        return 1;
    }

    Beatmup::PixelFormat fmt;
    if (!strToPixelFormat(argv[Args::OUTPUT_FORMAT], fmt)) {
        std::cerr << "Unknown output pixel format: " <<  argv[Args::OUTPUT_FORMAT] << std::endl;
        return 1;
    }

    Beatmup::Context ctx;

    const int
        outputWidth  = std::atoi(argv[Args::OUTPUT_WIDTH]),
        outputHeight = std::atoi(argv[Args::OUTPUT_HEIGHT]);
    Beatmup::InternalBitmap output(ctx, fmt, outputWidth, outputHeight);

    std::srand(std::time(nullptr));
    Beatmup::BitmapTools::noise(output);
    output.saveBmp(argv[Args::OUTPUT_FILENAME]);

    return 0;
}


int resample(int argc, const char* argv[]) {
    enum Args {
        INPUT_FILENAME = 1,
        METHOD,
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT,
        OUTPUT_FILENAME
    };

    if (argc <= Args::OUTPUT_FILENAME) {
        std::cerr << "Usage:" << std::endl
                << "    tools resample <input.bmp> nn|box|linear|cubic|convnet <width> <height> <output.bmp>" << std::endl;
        return 1;
    }

    // Create a context
    Beatmup::Context ctx;

    // Load input
    Beatmup::InternalBitmap input(ctx, argv[INPUT_FILENAME]);

    // Initialize the output bitmap
    const int
        outputWidth  = std::atoi(argv[Args::OUTPUT_WIDTH]),
        outputHeight = std::atoi(argv[Args::OUTPUT_HEIGHT]);
    Beatmup::InternalBitmap output(ctx, input.getPixelFormat(), outputWidth, outputHeight);

    // Instantiate a resampler
    Beatmup::BitmapResampler resampler;
    resampler.setBitmaps(&input, &output);

    if (strcmp(argv[Args::METHOD], "nn") == 0)
        resampler.setMode(Beatmup::BitmapResampler::Mode::NEAREST_NEIGHBOR);
    else if (strcmp(argv[Args::METHOD], "box") == 0)
        resampler.setMode(Beatmup::BitmapResampler::Mode::BOX);
    else if (strcmp(argv[Args::METHOD], "linear") == 0)
        resampler.setMode(Beatmup::BitmapResampler::Mode::LINEAR);
    else if (strcmp(argv[Args::METHOD], "cubic") == 0)
        resampler.setMode(Beatmup::BitmapResampler::Mode::CUBIC);
    else if (strcmp(argv[Args::METHOD], "convnet") == 0)
        resampler.setMode(Beatmup::BitmapResampler::Mode::CONVNET);
    else {
        std::cerr << "Unknown resampling method: " <<  argv[Args::METHOD] << std::endl;
        return 1;
    }

    // Process
    float time;
    std::cout << "Processing..." << std::endl;
    time = ctx.performTask(resampler);
    std::cout << time << " ms" << std::endl;

    // Save output to a file.
    std::cout << "Saving result to " << argv[Args::OUTPUT_FILENAME] << std::endl;
    output.saveBmp(argv[Args::OUTPUT_FILENAME]);

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        const char* tool = argv[1];
        const char** params = (const char**)(argv + 1);
        const int pcount = argc - 1;
        if (strcmp(tool, "measure") == 0)
            return measure(pcount, params);
        if (strcmp(tool, "noise") == 0)
            return noise(pcount, params);
        if (strcmp(tool, "resample") == 0)
            return resample(pcount, params);
    }

    std::cout << "Usage:" << std::endl
        << "    tools measure <norm> <input1.bmp> <input2.bmp>" << std::endl
        << "    tools noise <width> <height> <format> <output.bmp>" << std::endl
        << "    tools resample <input.bmp> <method> <width> <height> <output.bmp>" << std::endl
        << std::endl
        << "  Pixel formats:" << std::endl
        << "    bin           binary mask" << std::endl
        << "    quad          2-bit mask" << std::endl
        << "    hex           4-bit mask" << std::endl
        << "    lum           8-bit grayscale" << std::endl
        << "    rgb           24-bit RGB" << std::endl
        << "    rgba          32-bit RGBA" << std::endl
        << "    LUM           32-bit floating point grayscale" << std::endl
        << "    RGB           3x32-bit floating point RGB" << std::endl
        << "    RGBA          4x32-bit floating point RGBA" << std::endl;
    return 0;
}
