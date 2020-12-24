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

#include "classifier.h"
#include "conv2d.h"
#include "dense.h"
#include "pooling2d.h"
#include "softmax.h"

using namespace Beatmup;
using namespace NNets;


Classifier::Classifier(Context& context, ChunkCollection& data):
    Model(context),
    InferenceTask(*this, data),
    context(context)
{}


Classifier::~Classifier() {
    for (auto op : ops)
        delete op;
    ops.clear();
}


const std::vector<float>& Classifier::operator()(AbstractBitmap& input) {
    connect(input, *ops[0], 0);
    context.performTask(*this);
    return getProbabilities();
}


Job Classifier::start(AbstractBitmap& input) {
    connect(input, *ops[0], 0);
    return context.submitTask(*this);
}