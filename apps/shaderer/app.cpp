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
    Applying a custom shader to images
*/
#include <bitmap/internal_bitmap.h>
#include <shading/image_shader.h>
#include <shading/shader_applicator.h>
#include <iostream>


int main(int argc, char* argv[]) {
    enum Args {
        OUTPUT_WIDTH = 1,
        OUTPUT_HEIGHT,
        OUTPUT_FILENAME,
        INPUT_FILENAME,
        INPUT_VARIABLE
    };

    if (argc <= Args::OUTPUT_FILENAME || argc % 2 != 0) {
        std::cout << "Usage: shaderer <out w> <out h> <out filename> [<in filename> <variable> ...]" << std::endl;
        return 1;
    }

    // Create a context
    Beatmup::Context ctx;

    // Load input bitmaps
    std::vector<Beatmup::InternalBitmap*> inputs;
    for (int i = Args::INPUT_FILENAME; i < argc; i += 2) {
        std::cout << "Loading " << argv[i] << " (uniform \"" << argv[i + 1] << "\")" << std::endl;
        inputs.push_back(new Beatmup::InternalBitmap(ctx, argv[i]));
    }

    // Initialize the output bitmap
    // When running GLES 2.0, QuadByte is the optimal format to transfer from GPU memory.
    const int
        outputWidth  = std::atoi(argv[Args::OUTPUT_WIDTH]),
        outputHeight = std::atoi(argv[Args::OUTPUT_HEIGHT]);
    Beatmup::InternalBitmap output(ctx, Beatmup::PixelFormat::QuadByte, outputWidth, outputHeight);

    // Get shader code from stdin
    std::string line, code;
    while (std::getline(std::cin, line)) {
       code += line + '\n';
    }

    // Instantiate image shader, send the source code
    Beatmup::ImageShader shader(ctx);
    shader.setSourceCode(code);

    // Instantiate an applicator (a task to run the shader)
    Beatmup::ShaderApplicator applicator;
    applicator.setShader(&shader);
    applicator.setOutputBitmap(&output);

    // Add input images to the applicator
    for (size_t i = 0; i < inputs.size(); ++i) {
        applicator.addSampler(inputs[i], argv[Args::INPUT_VARIABLE + 2 * i]);
    }

    // Render
    float time;
    std::cout << "Rendering..." << std::endl;
    time = ctx.performTask(applicator);
    std::cout << "  First run: " << time << " ms" << std::endl;
    time = ctx.performTask(applicator);
    std::cout << "  Second run: " << time << " ms" << std::endl;
    // Second run is much likely faster: it has the shaders compiled and all the
    // bitmap data ready to be used by GPU. So will be any further render pass if
    // no new bitmaps/shaders are added.

    // Save output to a file.
    std::cout << "Saving rendered image to " << argv[Args::OUTPUT_FILENAME] << std::endl;
    output.saveBmp(argv[Args::OUTPUT_FILENAME]);

    // Cleaning up
    for (auto _ : inputs)
        delete _;

    return 0;
}