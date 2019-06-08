/*
	"BeatmupGL": platform-dependent OpenGL headers and auxiliary utilities
*/

#pragma once

#if BEATMUP_PLATFORM_WINDOWS
	#include <windows.h>
	#undef min
	#undef max
	#include <gl/glew.h>
	#include <gl/wglew.h>
	#include <gl/GL.h>
	#define BGL_TEXTURE_TARGET GL_TEXTURE_2D
#elif BEATMUP_PLATFORM_ANDROID
	#include <EGL/egl.h>
	#include <EGL/eglext.h>
#ifdef BEATMUP_OPENGLVERSION_GLES31
    #include <GLES3/gl31.h>
    #include <GLES3/gl3ext.h>
	#include <GLES2/gl2ext.h>
	#define BGL_TEXTURE_TARGET GL_TEXTURE_EXTERNAL_OES
#elif BEATMUP_OPENGLVERSION_GLES20
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
	#define BGL_TEXTURE_TARGET GL_TEXTURE_EXTERNAL_OES
#else
	#error OpenGL ES version is not specified
#endif
	#include <android/native_window_jni.h>
#endif

#include "../exception.h"

namespace Beatmup {
	namespace GL {
		class GLException : public Beatmup::Exception {
		public:
			GLException(const char* info, int errCode) : Exception("GL exception got: %s\nGL error %x", info, errCode) {}
			GLException(const char* info) : Exception("GL exception got: %s\nGL error %x", info, glGetError()) {}

			static inline void check(const char* info) {
				GLuint err = glGetError();
				if (err != GL_NO_ERROR)
					throw GLException(info, err);
			}
		};

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
			GL_BGRA,
#else
			GL_RGBA,		// QuadByte
#endif

#ifdef BEATMUP_OPENGLVERSION_GLES20
            GL_LUMINANCE,	// SingleFloat
#else
			GL_RED,			// SingleFloat
#endif
#ifdef BEATMUP_CHANNEL_ORDER_BGRA
            GL_BGR,		    // TripleFloat
#else
			GL_RGB,			// TripleFloat
#endif

#ifdef BEATMUP_CHANNEL_ORDER_BGRA
			GL_BGRA,
#else
            GL_RGBA         // QuadFloat
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
			GL_UNSIGNED_BYTE,	// QuadByte
			GL_FLOAT,			// SingleFloat
			GL_FLOAT,			// TripleFloat
			GL_FLOAT			// QuadFloat
		};
	}
}