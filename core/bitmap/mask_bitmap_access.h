/*
	Bitmap accessing utilities.
*/

#pragma once
#include "abstract_bitmap.h"
#include "../exception.h"
#include "pixel_arithmetic.h"

namespace Beatmup {
	/**
		A generic to access mask bitmap data
	*/
	template<const int num_bits> class MaskScanner {
	protected:
		unsigned char
			*data,				//!< all bitmap data
			*ptr,				//!< pointer to current pixel
			bit;				//!< current position bit
		int width, height;		//!< bitmap size in pixels

		MaskScanner(const AbstractBitmap& bitmap) {
#ifdef BEATMUP_DEBUG
			if (bitmap.getBitsPerPixel() != num_bits)
				BEATMUP_ERROR("Invalid mask bitmap scanner");
#endif
			width = bitmap.getWidth();
			height = bitmap.getHeight();
			data = (unsigned char*)bitmap.getData(0, 0);
		}

	public:
		typedef unsigned char pixvaltype;
		typedef pixint1 pixtype;

		const int
			NUMBER_OF_CHANNELS = 1,			// a mask always has one channel
			NUMBER_OF_BITS = num_bits,
			MAX_VALUE = 255,
			MAX_UNNORM_VALUE = (1 << num_bits) - 1;

		bool operator < (const MaskScanner& another) const {
			return (ptr < another.ptr) || (ptr == another.ptr && bit < another.bit);
		}

		pixtype* operator*() const {
			return (pixtype*)ptr;
		}

		/**
			Returns bitmap width in pixels
		*/
		inline int getWidth() const {
			return width;
		}

		/**
			Returns bitmap height in pixels
		*/
		inline int getHeight() const {
			return height;
		}
	};


	/**
		A generic to access sub-byte mask bitmap data
	*/
	template<const int num_bits, const int lookup[]> class LookupMaskScanner : public MaskScanner<num_bits> {
	protected:
		const int pointsPerByte = 8 / num_bits;
	public:
		/**
			Returns 0..MAX_UNNORM_VALUE value at current position
		*/
		inline unsigned char getValue() const {
			return ((*this->ptr) >> this->bit) & this->MAX_UNNORM_VALUE;
		}

		/**
			Returns 0..MAX_UNNORM_VALUE value at (x,y) position
		*/
		inline unsigned char getValue(int x, int y) const {
			msize n = (this->width*y + x);
			unsigned char
				*p = this->data + n / pointsPerByte,
				b = (unsigned char)(n % pointsPerByte);
			return ((*p) >> (b*num_bits)) & this->MAX_UNNORM_VALUE;
		}

		/**
			Returns 0..255 value at current position
		*/
		inline pixint1 operator()() const {
			return pixint1{ lookup[getValue()] };
		}

		/**
			Returns 0..255 value at (x,y) position
		*/
		inline pixint1 operator()(int x, int y) const {
			return pixint1{ lookup[getValue(x, y)] };
		}

		/**
			Move the current position ONE PIXEL forward
		*/
		inline void operator++(int)  {
			// int argument here is to declare the postfix increment (a C++ convention)
			if ((this->bit += num_bits) >= 8) {
				this->ptr++;
				this->bit = 0;
			}
		}

		/**
			Move the current position N pixels forward
		*/
		inline void operator+=(const int N)  {
			this->bit += N*num_bits;
			this->ptr += this->bit / 8;
			this->bit = this->bit % 8;
		}

		/**
			Changes current position
		*/
		inline void goTo(int x, int y) {
#ifdef BEATMUP_DEBUG
			if (x < 0 || y < 0 || x >= this->width || y >= this->height)
				BEATMUP_ERROR("Coordinates outside of image: %d %d (width=%d, height=%d)", x, y, this->width, this->height);
#endif
			msize n = (this->width*y + x);
			this->ptr = this->data + n / pointsPerByte,
			this->bit = (unsigned char)(n % pointsPerByte) * num_bits;
		}

		LookupMaskScanner(const AbstractBitmap& bitmap, int x = 0, int y = 0):
			MaskScanner<num_bits>(bitmap)
		{
			goTo(x, y);
		}
	};


	/**
		A generic to write mask bitmap data
	*/
	template<const int num_bits, const int lookup[]> class LookupMaskWriter : public LookupMaskScanner < num_bits, lookup >, BitmapContentModifier {
	public:
		/**
			Puts a properly scaled (0..MAX_UNNORM_VALUE) value at the current position
		*/
		inline void putValue(unsigned char x) {
			*this->ptr = (*this->ptr & ~(this->MAX_UNNORM_VALUE << this->bit)) + (x << this->bit);
		}

		/**
			Puts an unscaled (0..255) value at the current position.
			\return properly scaled value put into the mask.
		*/
		inline unsigned char assign(int x) {
			unsigned char v = x > 0 ? (x < 256 ? x * this->MAX_UNNORM_VALUE / 255 : this->MAX_UNNORM_VALUE) : 0;
			putValue(v);
			return v;
		}

		inline unsigned char assign(int r, int g, int b) {
			return assign((r + g + b) / 3);
		}

		inline unsigned char assign(int r, int g, int b, int a) {
			return assign(a);
		}

		inline unsigned char assign(float x) {
			unsigned char v = x > 0.0f ? (x < 1.0f ? (unsigned char)roundf(x * this->MAX_UNNORM_VALUE) : this->MAX_UNNORM_VALUE) : 0;
			putValue(v);
			return v;
		}

		inline unsigned char assign(float r, float g, float b) {
			return assign((r + g + b) / 3);
		}

		inline unsigned char assign(float r, float g, float b, float a) {
			return assign(a);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			*this->ptr = clipPixint(((P.r + P.g + P.b) * P.a + *this->ptr * (255 - P.a) * 3) / 765);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			*this->ptr = pixfloat2pixbyte((P.r + P.g + P.b) * P.a / 3 + *this->ptr * (1 - P.a));
		}

		inline void operator=(const pixint1& P) {
			assign(P.x);
		}

		inline void operator=(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator=(const pixint4& P) {
			assign(P.r, P.g, P.b, P.a);
		}

		inline void operator=(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator=(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator=(const pixfloat4& P) {
			assign(P.r, P.g, P.b, P.a);
		}

		LookupMaskWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : LookupMaskScanner < num_bits, lookup >(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};

	//!< lookup tables for masks values
	extern int MASK_LUT_1_BIT[2], MASK_LUT_2_BITS[4], MASK_LUT_4_BITS[16];

	typedef LookupMaskScanner<1, MASK_LUT_1_BIT> BinaryMaskReader;
	typedef LookupMaskScanner<2, MASK_LUT_2_BITS> QuaternaryMaskReader;
	typedef LookupMaskScanner<4, MASK_LUT_4_BITS> HexMaskReader;
	typedef LookupMaskWriter<1, MASK_LUT_1_BIT> BinaryMaskWriter;
	typedef LookupMaskWriter<2, MASK_LUT_2_BITS> QuaternaryMaskWriter;
	typedef LookupMaskWriter<4, MASK_LUT_4_BITS> HexMaskWriter;


	/**
		Mask reqding interface to single byte bitmap
	*/
	class SingleByteMaskReader : public MaskScanner<8> {
	public:
		/**
			Returns 0..MAX_UNNORM_VALUE value at current position
		*/
		inline unsigned char getValue() const {
			return *ptr;
		}

		/**
			Returns 0..MAX_UNNORM_VALUE value at (x,y) position
		*/
		inline unsigned char getValue(int x, int y) const {
			return data[width*y + x];
		}

		/**
			Returns 0..255 value at current position
		*/
		inline pixint1 operator()() const {
			return pixint1{ getValue() };
		}

		/**
			Returns 0..255 value at (x,y) position
		*/
		inline pixint1 operator()(int x, int y) const {
			return pixint1{ getValue(x, y) };
		}

		/**
			Move the current position ONE PIXEL forward
		*/
		inline void operator++(int)  {
			// int argument here is to declare the postfix increment (a C++ convention)
			ptr++;
		}

		/**
			Move the current position N pixels forward
		*/
		inline void operator+=(const int N)  {
			ptr += N;
		}

		/**
			Changes current position
		*/
		inline void goTo(int x, int y) {
#ifdef BEATMUP_DEBUG
			if (x < 0 || y < 0 || x >= width || y >= height)
				BEATMUP_ERROR("Coordinates outside of image: %d %d (width=%d, height=%d)", x, y, width, height);
#endif
			ptr = data + (width*y + x);
		}

		SingleByteMaskReader(const AbstractBitmap& bitmap, int x = 0, int y = 0):
			MaskScanner<8>(bitmap)
		{
			goTo(x, y);
		}
	};


	class SingleByteMaskWriter : public SingleByteMaskReader, BitmapContentModifier {
	public:
		/**
			Puts a properly scaled (0..MAX_UNNORM_VALUE) value at the current position
		*/
		inline void putValue(unsigned char x) {
			*this->ptr = x;
		}

		/**
			Puts an unscaled (0..255) value at the current position.
			\return properly scaled value put into the mask.
		*/
		inline unsigned char assign(int x) {
			putValue(x);
			return x;
		}

		inline unsigned char assign(int r, int g, int b) {
			return assign((r + g + b) / 3);
		}

		inline unsigned char assign(int r, int g, int b, int a) {
			return assign(a);
		}

		inline unsigned char assign(float x) {
			unsigned char v = x > 0.0f ? (x < 1.0f ? (unsigned char)roundf(x * this->MAX_UNNORM_VALUE) : this->MAX_UNNORM_VALUE) : 0;
			putValue(v);
			return v;
		}

		inline unsigned char assign(float r, float g, float b) {
			return assign((r + g + b) / 3);
		}

		inline unsigned char assign(float r, float g, float b, float a) {
			return assign(a);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			*this->ptr = clipPixint(((P.r + P.g + P.b) * P.a + *this->ptr * (255 - P.a) * 3) / 765);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			*this->ptr = pixfloat2pixbyte((P.r + P.g + P.b) * P.a / 3 + *this->ptr * (1 - P.a));
		}

		inline void operator=(const pixint1& P) {
			assign(P.x);
		}

		inline void operator=(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator=(const pixint4& P) {
			assign(P.r, P.g, P.b, P.a);
		}

		inline void operator=(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator=(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator=(const pixfloat4& P) {
			assign(P.r, P.g, P.b, P.a);
		}

		SingleByteMaskWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : SingleByteMaskReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};
}
