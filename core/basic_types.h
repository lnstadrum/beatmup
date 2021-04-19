/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

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
#include <cstdint>

namespace Beatmup {

#ifdef BEATMUP_PLATFORM_64BIT_
    const double pi = 3.141592653589793238462643383279502884;
    typedef uint64_t msize;			//!< memory size
    typedef uint64_t pixint_platform;
#else
    const float pi = 3.141592653589793238462643383279502884f;
    typedef uint32_t msize;				//!< memory size
    typedef uint32_t pixint_platform;
#endif

    typedef uint8_t pixbyte;
    typedef float pixfloat;

    typedef int dtime;                  //!< discrete time

    typedef struct {
        uint8_t r, g, b;
    } color3i;

    typedef struct {
        uint8_t r, g, b, a;
    } color4i;

    typedef struct {
        float r, g, b;
    } color3f;

    typedef struct {
        float r, g, b, a;
    } color4f;

    enum ProcessingTarget {CPU = 0, GPU};		//!< Where to process the stuff

    /**
        Low-level API providing access to GPU. Used internally.
    */
    namespace GL {
        typedef unsigned int handle_t;      //!< A reference to a GPU resource
    }

    /**
        %Beatmup object base class
    */
    class Object {
    public:
        virtual ~Object() {}
    };
}


/**
    Beatmup internal declarations.
    None of its contents is to be included in the application code.
*/
namespace Internal {}


/**
    Operations kernels.
    Used internally by Beatmup.
*/
namespace Kernels {}