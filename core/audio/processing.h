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
#include "sample_arithmetic.h"

namespace Beatmup {
    namespace Audio {
        /**
            Contains templates calling elementary audio signal processing routines depending on sample formats of their arguments.
            An elementary routine is a class template having a public function process() performing a specific processing action.
            The template arguments of this class are readers of / writers to signal specialized for given sample formats.
        */
        namespace Processing {

            template<template<typename, typename> class Func, typename... Args>
            inline void pipeline(const AudioSampleFormat inFormat, const AudioSampleFormat outFormat, const sample8* input, sample8* output, Args&&... args) {

#define WRITING(IN_T)  \
                switch (outFormat) { \
                    case Int8: \
                        Func<IN_T, sample8>::process((const IN_T*)input, (sample8*)output, args...); \
                        break; \
                    case Int16: \
                        Func<IN_T, sample16>::process((const IN_T*)input, (sample16*)output, args...); \
                        break; \
                    case Int32: \
                        Func<IN_T, sample32>::process((const IN_T*)input, (sample32*)output, args...); \
                        break; \
                    case Float32: \
                        Func<IN_T, sample32f>::process((const IN_T*)input, (sample32f*)output, args...); \
                        break; \
                }

                switch (inFormat) {
                case Int8:
                    WRITING(sample8);
                    break;
                case Int16:
                    WRITING(sample16);
                    break;
                case Int32:
                    WRITING(sample32);
                    break;
                case Float32:
                    WRITING(sample32f);
                    break;
                }
#undef WRITING
            }
        }
    }
}
