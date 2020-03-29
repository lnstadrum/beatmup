/**
    Resample a bitmap
*/
#include <bitmap/internal_bitmap.h>
#include <bitmap/resampler.h>
#include <iostream>


int main(int argc, char* argv[]) {
    enum Args {
        INPUT_FILENAME = 1,
        OUTPUT_WIDTH,
        OUTPUT_HEIGHT,
        OUTPUT_FILENAME
    };

    if (argc <= Args::OUTPUT_FILENAME) {
        std::cout << "Usage: resample <in filename> <out w> <out h> <out filename>" << std::endl;
        return 1;
    }

    // Create a context
    Beatmup::Context ctx;

  	// Load input
    Beatmup::InternalBitmap input(ctx, argv[INPUT_FILENAME]);

    // Initialize the output bitmap
    // When running GLES 2.0, QuadByte is the optimal format to transfer from GPU memory.
    const int
        outputWidth  = std::atoi(argv[Args::OUTPUT_WIDTH]),
        outputHeight = std::atoi(argv[Args::OUTPUT_HEIGHT]);
    Beatmup::InternalBitmap output(ctx, Beatmup::PixelFormat::QuadByte, outputWidth, outputHeight);

    // Instantiate a resampler
    Beatmup::BitmapResampler resampler;
    resampler.setBitmaps(&input, &output);
    resampler.setMode(Beatmup::BitmapResampler::Mode::CONVNET);

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
