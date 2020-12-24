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

#include "binding_tools.hpp"


py::object Beatmup::Python::getModelOutputDataByOp(NNets::Model& model, const NNets::AbstractOperation& operation, int output) {
    size_t numSamples;
    auto data = model.getOutputData(numSamples, operation, output);
    auto size = operation.getOutputSize(output);
    if (!data)
        return py::none();
    if (size[0] == 1 && size[2] == 1)
        // column vector got, return flat
        return py::array_t<float>({ size[1] }, { sizeof(float) }, data);
    else
        return py::array_t<float>(
            { size[1], size[0], size[2] },      // "H, W, C"
            { size[0] * size[2] * sizeof(float), size[2] * sizeof(float), sizeof(float) },
            data
        );
}


py::object Beatmup::Python::getModelOutputDataByName(NNets::Model& model, const std::string& opName, int output) {
    return getModelOutputDataByOp(model, model.getOperation(opName), output);
}