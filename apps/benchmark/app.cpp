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

/*
    Benchmarking GPU though linear mapping
*/

#include "context.h"
#include "gpu/linear_mapping.h"
#include "utils/progress_tracking.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>

using namespace Beatmup::GL;


/**
    Computes y = A*x + b for a random matrix A and random vectors x and b on GPU many times and meters the execution time.
*/
class BenchmarkTask : public Beatmup::GpuTask {
private:
    Beatmup::Context& context;
    const int width, height;
    const bool fixedPoint;

public:
    static const int DEFAULT_PROBLEM_SIZE = 1024;
    
    BenchmarkTask(Beatmup::Context& context, bool fixedPoint, const int width = DEFAULT_PROBLEM_SIZE, const int height = DEFAULT_PROBLEM_SIZE) :
        context(context), width(width), height(height), fixedPoint(fixedPoint)
    {}

    bool processOnGPU(Beatmup::GraphicPipeline& gpu, Beatmup::TaskThread& thread) {
        std::cout << "Benchmarking GPU by computing a " << width << "x" << height << " linear mapping." << std::endl;
        const auto format = fixedPoint ? Vector::Format::FIXED16 : Vector::DEFAULT_FORMAT;
        if (format == Vector::Format::FIXED16)
            std::cout << "Using 16-bit fixed-point computing." << std::endl;
        else
            std::cout << "Using floating-point computing (add -f option to force fixed-point computing)." << std::endl;
        std::cout << "GPU info: " << gpu.getGpuVendorString() << " " << gpu.getGpuRendererString() << std::endl;
        std::cout << "Preparing..." << std::endl;

        // make random matrix
        std::vector<float> matrix(width * height);
        std::random_device dev;
        std::default_random_engine dom(dev());
        {
            std::uniform_real_distribution<float> ran(-1, 1);
            for (auto& _ : matrix)
                _ = ran(dom);
        }

        // make random vector
        std::vector<float> vector(width);
        {
            std::uniform_int_distribution<int> ran(0, 255);
            for (auto& _ : vector)
                _ = ran(dom) / 255.0f;
        }

        // make random bias
        std::vector<float> bias(height);
        {
            std::uniform_real_distribution<float> ran(-1, 1);
            for (auto& _ : bias)
                _ = ran(dom);
        }

        // set up GL stuff
        Vector glVector(context, gpu, width, Vector::Format::TEXTURE, vector.data());
        Vector glResult(context, gpu, height, format);

        // compute ground truth
        std::vector<float> gt(height);
        for (size_t j = 0, k = 0; j < height; ++j) {
            gt[j] = bias[j];
            for (size_t i = 0; i < width; ++i, ++k)
                gt[j] += matrix[k] * vector[i];
        }

        // create mapping
        LinearMapping mapping(context, fixedPoint);
        mapping.setMatrix(gpu, width, height, matrix.data());
        mapping.setBias(gpu, height, bias.data());

        // run once
        mapping(gpu, glResult, glVector);
        gpu.flush();

        // run many times
        std::cout << "Run..." << std::endl;
        static const int maxNumIterations = 10000;
        uint64_t timeSum = 0, timeSqrSum = 0, timeMin = 0, timeMax = 0;
        int numIter = 0;
        for (Beatmup::ProgressTracking progress(maxNumIterations); !progress.done(); progress()) {
            auto start = std::chrono::high_resolution_clock::now();
            mapping(gpu, glResult, glVector);
            gpu.flush();
            auto stop = std::chrono::high_resolution_clock::now();

            uint64_t duration = (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
            timeSum += duration;
            timeSqrSum += duration * duration;

            if (timeMin == 0 && timeMax == 0)
                timeMin = timeMax = duration;
            else {
                timeMin = std::min(timeMin, duration);
                timeMax = std::max(timeMax, duration);
            }

            numIter++;
            if (numIter % 100 == 0)
                progress.printOutProgressBar();
        }
        std::cout << std::endl << std::endl;

        // check the result
        std::vector<float> result;
        glResult.fetch(gpu, result);
        float err = 0;
        for (size_t i = 0; i < result.size(); ++i)
            err = std::max(err, std::abs(gt[i] - result[i]));
        std::cout << "Max abs error: " << std::defaultfloat << err << std::endl;

        // print stats
        const double
            avg = (double)timeSum / numIter,
            stdev = std::sqrt(timeSqrSum / numIter - avg * avg);

        std::cout << "Iterations done: " << numIter << std::endl;
        std::cout << "Average time per run: " << avg << " us (" << stdev << " us std)" << std::endl;
        std::cout << "Slowest run:          " << timeMax << " us" << std::endl;
        std::cout << "Fastest run:          " << timeMin << " us" << std::endl << std::endl;

        const int multiplyAdds = width * height;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Average speed: " << multiplyAdds / avg << " Mmadds/s" << std::endl;
        std::cout << "Highest speed: " << multiplyAdds / timeMin << " Mmadds/s" << std::endl;
        std::cout << "Lowest speed:  " << multiplyAdds / timeMax << " Mmadds/s" << std::endl << std::endl;

        return true;
    }
};


int main(int argc, const char* argv[]) {
#ifdef BEATMUP_OPENGLVERSION_GLES20
    const bool fixedPoint = true;
#else
    const std::string forceFixedOption = "-f";
    const bool fixedPoint = argc >= 2 && forceFixedOption == argv[1] ? true : false;
#endif

    Beatmup::Context context;
    BenchmarkTask benchmark(context, fixedPoint);
    context.performTask(benchmark);

    return 0;
}
