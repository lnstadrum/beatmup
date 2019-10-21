/*
	Basic datatypes
*/

#pragma once
#include <stdint.h>

#define ceili(x,y) (((x)+(y)-1)/(y))				//!< integer division x/y with ceiling
#define roundf_fast(X) (floorf_fast((X) + 0.5f))	//!< rounding (nearest integer)
#define clamp(X0,X,X1) (std::min(std::max((X0),(X)), (X1)))

// fast floorf (no range check)
inline int floorf_fast(float x) {
	int i = (int)x;
	return i - (i > x);
}

// modulus
inline float modf(float x, float y) {
	return x - y * (int)(x / y);
}


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

	typedef int dtime;

	enum ProcessingTarget {CPU = 0, GPU};		//!< where to process the stuff

	namespace GL {
		typedef unsigned int glhandle;
	}

	/**
		Object base
	*/
	class Object {
	public:
		virtual ~Object() {}
	};
}