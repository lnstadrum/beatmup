/**
    Upsamples an image using a convolutional neural net inferred on GPU
*/
#include <bitmap/internal_bitmap.h>
#include <bitmap/resampler.h>
#include <iostream>


int main(int argc, char* argv[]) {
    enum Args {
        INPUT_FILENAME = 1,
        OUTPUT_FILENAME
    };

    if (argc <= Args::OUTPUT_FILENAME) {
        std::cout
            << "Usage: resample <input file> <output file>" << std::endl
            << "BMP files are only supported." << std::endl
            << std::endl;
        return 1;
    }

    // Create a context
    Beatmup::Context ctx;

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
    Beatmup::BitmapResampler resampler;
    resampler.setMode(Beatmup::BitmapResampler::Mode::CONVNET);

    // Run the task on dummy bitmaps first to warm up GPU and compile shaders
    std::cout << "Preparing GPU... " << std::endl;
    float time;
    Beatmup::InternalBitmap warmupIn(ctx, Beatmup::PixelFormat::QuadByte, 64, 64), warmupOut(ctx, Beatmup::PixelFormat::QuadByte, 128, 128);
    resampler.setBitmaps(&warmupIn, &warmupOut);
    time = ctx.performTask(resampler);
    std::cout << time << " ms" << std::endl;

    // Run the inference
    std::cout << "Processing... " << std::endl;
    resampler.setBitmaps(&input, &output);
    time = ctx.performTask(resampler);
    std::cout << time << " ms" << std::endl;

    // Save output to a file.
    std::cout << "Saving result to " << argv[Args::OUTPUT_FILENAME] << std::endl;
    output.saveBmp(argv[Args::OUTPUT_FILENAME]);

    return 0;
}
