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

#pragma once

#include "geometry.h"
#include "bitmap/pixel_arithmetic.h"
#include "nnets/model.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

namespace Beatmup {
    namespace Python {

        template<typename T>
        inline CustomPoint<T> toPoint(const py::tuple& tuple) {
            if (tuple.size() != 2)
                throw std::invalid_argument("Expected a tuple of 2 elements");
            return CustomPoint<T>(tuple[0].cast<T>(), tuple[1].cast<T>());
        }

        template<typename T>
        inline CustomRectangle<T> toRectangle(const py::tuple& tuple) {
            if (tuple.size() != 4)
                throw std::invalid_argument("Expected a tuple of 4 elements");
            return CustomRectangle<T>(tuple[0].cast<T>(), tuple[1].cast<T>(), tuple[2].cast<T>(), tuple[3].cast<T>());
        }


        template<typename T>
        inline py::tuple toTuple(const CustomPoint<T>& point) {
            return py::make_tuple(point.x, point.y);
        }

        template<typename T>
        inline py::tuple toTuple(const CustomRectangle<T>& rectangle) {
            return py::make_tuple(rectangle.a.x, rectangle.a.y, rectangle.b.x, rectangle.b.y);
        }

        template<typename T>
        inline py::tuple toTuple(const CustomMatrix2<T>& matrix) {
            return py::make_tuple(
                py::make_tuple(matrix.getA11(), matrix.getA12()),
                py::make_tuple(matrix.getA21(), matrix.getA22())
            );
        }

        inline py::tuple toTuple(const color3f& color) {
            return py::make_tuple(color.r, color.g, color.b);
        }

        inline py::tuple toTuple(const color4i& color) {
            return py::make_tuple(color.r, color.g, color.b, color.a);
        }

        inline pixfloat4 toPixfloat4(const py::tuple& tuple) {
            if (tuple.size() != 4)
                throw std::invalid_argument("Expected a tuple of 4 elements");
            return pixfloat4(tuple[0].cast<float>(), tuple[1].cast<float>(), tuple[2].cast<float>(), tuple[3].cast<float>());
        }

        inline color3f toColor3f(const py::tuple& tuple) {
            if (tuple.size() != 3)
                throw std::invalid_argument("Expected a tuple of 3 elements");
            return color3f{ tuple[0].cast<float>(), tuple[1].cast<float>(), tuple[2].cast<float>() };
        }

        inline color4f toColor4f(const py::tuple& tuple) {
            if (tuple.size() != 4)
                throw std::invalid_argument("Expected a tuple of 4 elements");
            return color4f{ tuple[0].cast<float>(), tuple[1].cast<float>(), tuple[2].cast<float>(), tuple[3].cast<float>() };
        }

        inline color4i toColor4i(const py::tuple& tuple) {
            if (tuple.size() != 4)
                throw std::invalid_argument("Expected a tuple of 4 elements");
            return color4i{ tuple[0].cast<uint8_t>(), tuple[1].cast<uint8_t>(), tuple[2].cast<uint8_t>(), tuple[3].cast<uint8_t>() };
        }


        py::object getModelOutputDataByName(NNets::Model& model, const std::string& opName, int output);

        py::object getModelOutputDataByOp(NNets::Model& model, const NNets::AbstractOperation& operation, int output);

    }
}