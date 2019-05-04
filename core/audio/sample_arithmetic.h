/*
	Audio sample values arithmetic
*/
#pragma once
#include "../basic_types.h"

namespace Beatmup {

	enum AudioSampleFormat {
		Int8,
		Int16,
		Int32,
		Float32
	};

	const int AUDIO_SAMPLE_SIZE[] = { 1, 2, 3, 4 };
	const char* const AUDIO_FORMAT_NAME[] = { "8 bit", "16 bit", "32 bit", "32 bit float" };

	struct sample8;
	struct sample16;
	struct sample32;
	struct sample32f;
	typedef sample8* psample;

	struct sample8 {
		signed char x;
		inline bool operator < (const sample8& sample) const { return x < sample.x; }
		operator sample16() const;
		operator sample32() const;
		operator sample32f() const;
		void operator =(const sample16 S);
		void operator =(const sample32 S);
		void operator =(const sample32f S);
		static const int
			MIN_VALUE = -128,
			MAX_VALUE = +127;
	};

	struct sample16 {
		signed short int x;
		inline bool operator < (const sample16& sample) const { return x < sample.x; }
		operator sample8() const;
		operator sample32() const;
		operator sample32f() const;
		void operator =(const sample8 S);
		void operator =(const sample32 S);
		void operator =(const sample32f S);
		static const int
			MIN_VALUE = -32768,
			MAX_VALUE = +32767;
	};

	struct sample32 {
		int x;
		inline bool operator < (const sample32& sample) const { return x < sample.x; }
		operator sample8() const;
		operator sample16() const;
		operator sample32f() const;
		void operator =(const sample8 S);
		void operator =(const sample16 S);
		void operator =(const sample32f S);
		static const long
			MIN_VALUE = -2147483647-1,
			MAX_VALUE = +2147483647;
	};

	struct sample32f {
		float x;
		inline bool operator < (const sample32f& sample) const { return x < sample.x; }
		operator sample8() const;
		operator sample16() const;
		operator sample32() const;
		void operator =(const sample8 S);
		void operator =(const sample16 S);
		void operator =(const sample32 S);
		static const float
			MIN_VALUE,
			MAX_VALUE;
	};

	//////////////////////////////////////////////////////////////
	//						8 bit integer						//
	//////////////////////////////////////////////////////////////
	
	inline sample8::operator sample16() const {
		return sample16{ (decltype(sample16::x)) (x << 8) };
	}

	inline sample8::operator sample32() const {
		return sample32{ x << 2 };
	}

	inline sample8::operator sample32f() const {
		return sample32f{ x / 127.0f };
	}

	inline void sample8::operator =(const sample16 S) {
		x = (decltype(x))( S.x >> 8 );
	}

	inline void sample8::operator =(const sample32 S) {
		x = (decltype(x))( S.x >> 24 );
	}

	inline void sample8::operator =(const sample32f S) {
		x = (decltype(x)) roundf_fast(S.x * 127);
	}


	//////////////////////////////////////////////////////////////
	//						16 bit integer						//
	//////////////////////////////////////////////////////////////
	
	inline sample16::operator sample8() const {
		return sample8{ (decltype(sample8::x))( x >> 8 ) };
	}

	inline sample16::operator sample32() const {
		return sample32{ x << 16 };
	}

	inline sample16::operator sample32f() const {
		return sample32f{ x / 32767.0f };
	}

	inline void sample16::operator =(const sample8 S) {
		x = S.x << 8;
	}
	
	inline void sample16::operator =(const sample32 S) {
		x = (decltype(x))( S.x >> 8 );
	}

	inline void sample16::operator =(const sample32f S) {
		x = (decltype(x)) roundf_fast(S.x * 32767);
	}

	//////////////////////////////////////////////////////////////
	//						32 bit integer						//
	//////////////////////////////////////////////////////////////
	
	inline sample32::operator sample8() const {
		return sample8{ (decltype(sample8::x)) (x >> 24) };
	}

	inline sample32::operator sample16() const {
		return sample16{ (decltype(sample16::x)) (x >> 16) };
	}

	inline sample32::operator sample32f() const {
		return sample32f{ x / 2147483647.0f };
	}

	inline void sample32::operator =(const sample8 S) {
		x = S.x << 24;
	}

	inline void sample32::operator =(const sample16 S) {
		x = S.x << 16;
	}

	inline void sample32::operator =(const sample32f S) {
		x = roundf_fast(S.x * 2147483647);
	}

	//////////////////////////////////////////////////////////////
	//						32 bit float						//
	//////////////////////////////////////////////////////////////
	
	inline sample32f::operator sample8() const {
		return sample8{ (decltype(sample8::x)) roundf_fast(x *127) };
	}

	inline sample32f::operator sample16() const {
		return sample16{ (decltype(sample16::x)) roundf_fast(x * 32767) };
	}

	inline sample32f::operator sample32() const {
		return sample32{ roundf_fast(x * 2147483647) };
	}

	inline void sample32f::operator =(const sample8 S) {
		x = S.x / 128.0f;
	}

	inline void sample32f::operator =(const sample16 S) {
		x = S.x / 32768.0f;
	}

	inline void sample32f::operator =(const sample32 S) {
		x = S.x / 2147483648.0f;
	}

	//////////////////////////////////////////////////////////////
	//						Miscellaneous						//
	//////////////////////////////////////////////////////////////

	inline bool operator!=(const sample16& A, const sample16& B) {
		return A.x != B.x;
	}


	template<typename in, typename out> void convertSamples(in* const input, void* output, msize count) {
		const in* stop = input + count;
		out* po = (out*)output;
		for (const in* pi = input; pi < stop; pi++, po++)
			*po = *pi;
	}
}