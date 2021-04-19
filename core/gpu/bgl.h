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

#ifdef BEATMUP_OPENGLVERSION_GLES
    #include <EGL/egl.h>
    #include <EGL/eglext.h>
  #ifdef BEATMUP_OPENGLVERSION_GLES31
    #include <GLES3/gl31.h>
    #include <GLES3/gl3ext.h>
    #include <GLES2/gl2ext.h>
  #elif BEATMUP_OPENGLVERSION_GLES20
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
  #else
    #error OpenGL ES version is not specified
  #endif
  #ifdef BEATMUP_PLATFORM_ANDROID
    #include <android/native_window_jni.h>
  #endif
#elif BEATMUP_PLATFORM_WINDOWS
    #include <windows.h>
    #undef min
    #undef max
    #include <gl/glew.h>
    #include <gl/wglew.h>
#else
    #include <X11/Xlib.h>
    #include <GL/glxew.h>
#endif

#include "../exception.h"

namespace Beatmup {
    namespace GL {

        /**
            GPU exception
        */
        class GLException : public Beatmup::Exception {
        public:
            GLException(const char* info, int errCode) : Exception("GL error %x: %s", errCode, info) {}
            GLException(const char* info) : Exception("GL error %x: %s", glGetError(), info) {}
            GLException(const std::string& info) : GLException(info.c_str()) {}

            static inline void check(const std::string& info) {
                GLuint err = glGetError();
                if (err != GL_NO_ERROR)
                    throw GLException(info.c_str(), err);
            }
        };

        /**
            Exception communicating the use of an unsupported feature on GPU
        */
        class Unsupported : public Beatmup::Exception {
        public:
            Unsupported(const char* info) : Exception(info) {}
        };

        /**
            Mapping of bitmap pixel formats to GL pixel formats
        */
        const GLuint BITMAP_PIXELFORMATS[] = {
#ifdef BEATMUP_OPENGLVERSION_GLES20
            GL_LUMINANCE,	// SingleByte
#else
            GL_RED,			// SingleByte
#endif


#ifdef BEATMUP_CHANNEL_ORDER_BGRA
            GL_BGR,			// TripleByte
#else
            GL_RGB,			// TripleByte
#endif


#ifdef BEATMUP_CHANNEL_ORDER_BGRA
            GL_BGRA,		// QuadByte
#elif BEATMUP_CHANNEL_ORDER_ARGB
            GL_BGRA,		// QuadByte; an inversion is done in a modern OpenGL using the pixeltype trick
#else
            GL_RGBA,		// QuadByte
#endif


#ifdef BEATMUP_OPENGLVERSION_GLES20
            GL_LUMINANCE,	// SingleFloat
#else
            GL_RED,			// SingleFloat
#endif


#ifdef BEATMUP_CHANNEL_ORDER_BGRA
            GL_BGR,			// TripleFloat
#else
            GL_RGB,			// TripleFloat
#endif


#ifdef BEATMUP_CHANNEL_ORDER_BGRA
            GL_BGRA			// QuadFloat
#elif BEATMUP_CHANNEL_ORDER_ARGB
            GL_BGRA			// QuadFloat
#else
            GL_RGBA			// QuadFloat
#endif
        };


        const GLuint TEXTUREHANDLER_INTERNALFORMATS[] = {
#ifdef BEATMUP_OPENGLVERSION_GLES20
            GL_LUMINANCE,	// Rx8
            GL_RGB,			// RGBx8
            GL_RGBA,		// RGBAx8
            GL_LUMINANCE,	// Rx32f
            GL_RGB,			// RGBx32f
            GL_RGBA, 		// RGBAx32f
#else
            GL_R8,			// Rx8
            GL_RGB8,		// RGBx8
            GL_RGBA8,		// RGBAx8
            GL_R32F,    	// Rx32f
            GL_RGB32F,	    // RGBx32f
            GL_RGBA32F,     // RGBAx32f
#endif
            0
        };

        const GLuint BITMAP_INTERNALFORMATS[] = {
            TEXTUREHANDLER_INTERNALFORMATS[0],
            TEXTUREHANDLER_INTERNALFORMATS[1],
            TEXTUREHANDLER_INTERNALFORMATS[2],
            TEXTUREHANDLER_INTERNALFORMATS[3],
            TEXTUREHANDLER_INTERNALFORMATS[4],
            TEXTUREHANDLER_INTERNALFORMATS[5]
        };


        /**
            Mapping of bitmap pixel formats to GL pixel types
        */
        const GLuint BITMAP_PIXELTYPES[] = {
            GL_UNSIGNED_BYTE,	// SingleByte

            GL_UNSIGNED_BYTE,	// TripleByte

#if BEATMUP_CHANNEL_ORDER_ARGB	// QuadByte, special case of ARGB first
    #ifdef BEATMUP_OPENGLVERSION_GLES
            GL_UNSIGNED_BYTE,		// no way to do in OpenGL ES...
    #else
        // QuadByte, inverting BGRA by applying appropriate pixel type. The latter
        // depends on the current platform endianness.
        #ifdef BEATMUP_PLATFORM_BIGENDIAN
            GL_UNSIGNED_INT_8_8_8_8_REV,
        #else
            GL_UNSIGNED_INT_8_8_8_8,
        #endif
    #endif
#else
            GL_UNSIGNED_BYTE,	// QuadByte, other pixel orders
#endif

            GL_FLOAT,			// SingleFloat

            GL_FLOAT,			// TripleFloat

            GL_FLOAT			// QuadFloat
        };
    }
}
