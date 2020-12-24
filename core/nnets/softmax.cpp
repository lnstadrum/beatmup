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

#include "softmax.h"
#include "deserialized_model.h"

using namespace Beatmup;
using namespace NNets;


Softmax::Softmax(const std::string& name): CpuOperation(name) {}


int Softmax::getAmountOfWork() const {
    return (int)output.size();
}


void Softmax::beforeExecute(GraphicPipeline& gpu, const int threadCount) {
    partialSums.resize(threadCount);
    if (inputVector)
        inputVector->fetch(gpu, output);
    else {
        inputView.getStorage().pull(gpu);
        output.resize(inputView.getSize().volume());
        Storage::Scanner scanner(inputView);
        scanner.move(0, 0);
        scanner.fill(output.begin(), output.end());
    }
}


void Softmax::execute(const int sliceStart, const int sliceStop, const int threadIdx, const int threadCount) {
    // compute the exponential mapping of the input vector and partial sums
    float sum = 0;
    for (int i = sliceStart; i < sliceStop; ++i)
        sum += (output[i] = std::exp(output[i]));
    partialSums[threadIdx] = sum;
}


void Softmax::afterExecute(const int threadCount) {
    // sum up partial sums
    float sum = partialSums[0];
    for (size_t i = 1; i < partialSums.size(); ++i)
        sum += partialSums[i];

    // normalize the output vector
    const float normFactor = 1 / sum;
    for (auto& y : output)
        y *= normFactor;
}


void Softmax::setInput(Storage::View&& view, int index) {
    OutOfRange::check(index, 0, 1, "Softmax operation input out of range: %d");
    InvalidArgument::check(view.getWidth() == 1 && view.getHeight() == 1, "A column-like storage view is expected on input of Softmax operation.");
    this->inputView = std::move(view);
    this->inputVector = nullptr;
}


void Softmax::setInput(GL::Vector& vector, int index) {
    OutOfRange::check(index, 0, 1, "Softmax operation input out of range: %d");
    this->inputVector = &vector;
    this->inputView = Storage::View();
}


bool Softmax::initDeserializer() {
    static class SoftmaxDeserializer : public AbstractOperation::Deserializer {
    public:
        SoftmaxDeserializer() : Deserializer("softmax") {}

        AbstractOperation* deserialize(Context& context, const Listing::Block& block) {
            /** \page NNetsOpsSerialization
                \section Softmax
                Softmax operation has no parameters (except its name and the type).
                \code{yaml}
                - _name: arbitrary operation name
                  _type: softmax    # fixed string
                \endcode
            */
            return new Softmax(block["_name"]);
        }
    } john;
    return true;
}


std::map<std::string, std::string> Softmax::serialize() const {
    return {
        { "_name", getName() },
        { "_type", "softmax" }
    };
}


void Softmax::disconnect() {
    this->inputVector = nullptr;
    this->inputView = Storage::View();
}