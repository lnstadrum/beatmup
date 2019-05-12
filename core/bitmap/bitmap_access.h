/*
	Bitmap accessing utilities.
	For performance reasons the implementation is inline-forced.
*/

#pragma once
#include "abstract_bitmap.h"
#include "../exception.h"
#include "pixel_arithmetic.h"

/**
 * Color channel order specification
 */
static const struct {
	const int A, R, G, B;
} CHANNELS = {
#ifdef BEATMUP_CHANNEL_ORDER_ARGB
	0, 1, 2, 3
#elif BEATMUP_CHANNEL_ORDER_BGRA
	3, 2, 1, 0
#else // rgba
	3, 0, 1, 2
#endif
};

namespace Beatmup {
	class BitmapContentModifier {
	protected:
		BitmapContentModifier(AbstractBitmap& bitmap);
	};

	/**
		A generic to access bitmap data
	*/
	template<typename pixel, const int num_channels> class CustomBitmapScanner {
	protected:
		pixel* data;			//!< bitmap data
		pixel* ptr;				//!< pointer to the current pixel
		int width, height;		//!< bitmap sizes in pixels
		
		/**
			Retrieves pixel address at a given position;
		*/
		inline pixel* jump(int x, int y) const {
			return data + num_channels*(width*y + x);
		}
	public:
		typedef pixel pixvaltype;

		const int NUMBER_OF_CHANNELS = num_channels;

		bool operator < (const CustomBitmapScanner& another) const {
			return ptr < another.ptr;
		}
		
		pixel* operator*() const {
			return ptr;
		}

		/**
			Move the current position ONE PIXEL forward
		*/
		inline void operator++(int)  {
			// int argument here is to declare the postfix increment (a C++ convention)
			ptr += num_channels;
		}

		/**
			Move the current position N pixels forward
		*/
		inline void operator+=(const int n)  {
			ptr += num_channels * n;
		}

		/**
			Changes current position
		*/
		inline void goTo(int x, int y) {
#ifdef BEATMUP_DEBUG
			if (x < 0 || y < 0 || x >= width || y >= height)
				BEATMUP_ERROR("Coordinates outside of image: %d %d (width=%d, height=%d)", x, y, width, height);
#endif
			ptr = data + num_channels*(width*y + x);
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

		CustomBitmapScanner(const AbstractBitmap& bitmap, int x = 0, int y = 0) {
#ifdef BEATMUP_DEBUG
			if (bitmap.getBitsPerPixel() != 8* num_channels * sizeof(pixel))
				BEATMUP_ERROR("Invalid bitmap scanner");
#endif
			width = bitmap.getWidth();
			height = bitmap.getHeight();
			data = (pixel*)bitmap.getData(0, 0);
			goTo(x, y);
		}
	};

	/**
		Single byte bitmap reader
	*/
	class SingleByteBitmapReader : public CustomBitmapScanner<pixbyte, 1> {
	public:
		typedef pixint1 pixtype;

		const int MAX_VALUE = 255;

		/**
			Returns value at current position
		*/
		inline pixint1 operator()() const {
			return pixint1{ *ptr };
		}

		/**
			Returns value at pixel (x,y) position
		*/
		inline pixint1 operator()(int x, int y) const {
			return pixint1{ data[y * width + x] };
		}

		SingleByteBitmapReader(const AbstractBitmap& bitmap, int x = 0, int y = 0) : CustomBitmapScanner(bitmap, x, y) {}
	};


	/**
		Triple byte bitmap reader
	*/
	class TripleByteBitmapReader : public CustomBitmapScanner<pixbyte, 3> {
	public:
		typedef pixint3 pixtype;

		const int MAX_VALUE = 255;

		inline pixint3 operator()() const {
			return pixint3{ ptr[CHANNELS.R], ptr[CHANNELS.G], ptr[CHANNELS.B] };
		}

		inline pixint3 operator()(int x, int y) const {
			int i = 3 * (y * width + x);
			return pixint3{ data[i + CHANNELS.R], data[i + CHANNELS.G], data[i + CHANNELS.B] };
		}

		TripleByteBitmapReader(const AbstractBitmap& bitmap, int x = 0, int y = 0) : CustomBitmapScanner(bitmap, x, y) {}
	};


	/**
		Quad byte bitmap reader
	*/
	class QuadByteBitmapReader : public CustomBitmapScanner<pixbyte, 4> {
	public:
		typedef pixint4 pixtype;

		const int MAX_VALUE = 255;

		inline pixint4 operator()() const {
			return pixint4(ptr[CHANNELS.R], ptr[CHANNELS.G], ptr[CHANNELS.B], ptr[CHANNELS.A]);
		}

		inline pixint4 operator()(int x, int y) const {
			int i = 4 * (y * width + x);
			return pixint4(data[i + CHANNELS.R], data[i + CHANNELS.G], data[i + CHANNELS.B], data[i + CHANNELS.A]);
		}

		QuadByteBitmapReader(const AbstractBitmap& bitmap, int x = 0, int y = 0) : CustomBitmapScanner(bitmap, x, y) {}
	};


	/**
		Single float bitmap reader
	*/
	class SingleFloatBitmapReader : public CustomBitmapScanner<pixfloat, 1> {
	public:
		typedef pixfloat1 pixtype;

		const float MAX_VALUE = 1.0f;

		inline pixfloat1 operator()() const {
			return pixfloat1{ *ptr };
		}

		inline pixfloat1 operator()(int x, int y) const {
			return pixfloat1{ data[y * width + x] };
		}

		SingleFloatBitmapReader(const AbstractBitmap& bitmap, int x = 0, int y = 0) : CustomBitmapScanner(bitmap, x, y) {}
	};


	/**
		Triple float bitmap reader
	*/
	class TripleFloatBitmapReader : public CustomBitmapScanner<pixfloat, 3> {
	public:
		typedef pixfloat3 pixtype;

		const float MAX_VALUE = 1.0f;

		inline pixfloat3 operator()() const {
			return pixfloat3{ ptr[CHANNELS.R], ptr[CHANNELS.G], ptr[CHANNELS.B] };
		}

		inline pixfloat3 operator()(int x, int y) const {
			int i = 3 * (y * width + x);
			return pixfloat3{ data[i + CHANNELS.R], data[i + CHANNELS.G], data[i + CHANNELS.B] };
		}

		TripleFloatBitmapReader(const AbstractBitmap& bitmap, int x = 0, int y = 0) : CustomBitmapScanner(bitmap, x, y) {}
	};


	/**
		Quad float bitmap reader
	*/
	class QuadFloatBitmapReader : public CustomBitmapScanner<pixfloat, 4> {
	public:
		typedef pixfloat4 pixtype;

		const float MAX_VALUE = 1.0f;

		inline pixfloat4 operator()() const {
			return pixfloat4(ptr[CHANNELS.R], ptr[CHANNELS.G], ptr[CHANNELS.B], ptr[CHANNELS.A]);
		}

		inline pixfloat4 operator()(int x, int y) const {
			int i = 4 * (y * width + x);
			return pixfloat4(data[i + CHANNELS.R], data[i + CHANNELS.G], data[i + CHANNELS.B], data[i + CHANNELS.A]);
		}

		inline pixfloat4 at(int x, int y) const {
			int i = 4 * (y * width + x);
			return pixfloat4(ptr[i + CHANNELS.R], ptr[i + CHANNELS.G], ptr[i + CHANNELS.B], ptr[i + CHANNELS.A]);
		}

		QuadFloatBitmapReader(const AbstractBitmap& bitmap, int x = 0, int y = 0) : CustomBitmapScanner(bitmap, x, y) {}
	};


	/**
		Single byte bitmap writer
	*/
	class SingleByteBitmapWriter : public SingleByteBitmapReader, BitmapContentModifier {
	public:
		inline void assign(int x) {
			*ptr = clipPixint(x);
		}

		inline void assign(int r, int g, int b) {
			*ptr = clipPixint((r + g + b) / 3);
		}

		inline void assign(int r, int g, int b, int a) {
			*ptr = clipPixint((r + g + b) / 3);
		}

		inline void assign(float x) {
			*ptr = pixfloat2pixbyte(x);
		}

		inline void assign(float r, float g, float b) {
			*ptr = pixfloat2pixbyte((r + g + b) / 3);
		}

		inline void assign(float r, float g, float b, float a) {
			*ptr = pixfloat2pixbyte((r + g + b) / 3);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			*ptr = clipPixint(((P.r + P.g + P.b) * P.a + *ptr * (255 - P.a) * 3) / 765);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			*ptr = pixfloat2pixbyte((P.r + P.g + P.b) * P.a / 3 + *ptr * (1 - P.a));
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

		SingleByteBitmapWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : SingleByteBitmapReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};

	
	/**
		Triple byte bitmap writer
	*/
	class TripleByteBitmapWriter : public TripleByteBitmapReader, BitmapContentModifier {
	public:
		inline void assign(int x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = clipPixint(x);
		}

		inline void assign(int r, int g, int b) {
			ptr[CHANNELS.R] = clipPixint(r);
			ptr[CHANNELS.G] = clipPixint(g);
			ptr[CHANNELS.B] = clipPixint(b);
		}

		inline void assign(int r, int g, int b, int a) {
			ptr[CHANNELS.R] = clipPixint(r);
			ptr[CHANNELS.G] = clipPixint(g);
			ptr[CHANNELS.B] = clipPixint(b);
		}

		inline void assign(float x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = pixfloat2pixbyte(x);
		}

		inline void assign(float r, float g, float b) {
			ptr[CHANNELS.R] = pixfloat2pixbyte(r);
			ptr[CHANNELS.G] = pixfloat2pixbyte(g);
			ptr[CHANNELS.B] = pixfloat2pixbyte(b);
		}

		inline void assign(float r, float g, float b, float a) {
			ptr[CHANNELS.R] = pixfloat2pixbyte(r);
			ptr[CHANNELS.G] = pixfloat2pixbyte(g);
			ptr[CHANNELS.B] = pixfloat2pixbyte(b);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			ptr[CHANNELS.R] = clipPixint((P.r * P.a + ptr[CHANNELS.R] * (255 - P.a)) / 255);
			ptr[CHANNELS.G] = clipPixint((P.g * P.a + ptr[CHANNELS.G] * (255 - P.a)) / 255);
			ptr[CHANNELS.B] = clipPixint((P.b * P.a + ptr[CHANNELS.B] * (255 - P.a)) / 255);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			float _a = (1 - P.a) / 255;
			ptr[CHANNELS.R] = pixfloat2pixbyte(P.r * P.a + ptr[CHANNELS.R] * _a);
			ptr[CHANNELS.G] = pixfloat2pixbyte(P.g * P.a + ptr[CHANNELS.G] * _a);
			ptr[CHANNELS.B] = pixfloat2pixbyte(P.b * P.a + ptr[CHANNELS.B] * _a);
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

		TripleByteBitmapWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : TripleByteBitmapReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};

	
	/**
		Quad byte bitmap writer
	*/
	class QuadByteBitmapWriter : public QuadByteBitmapReader, BitmapContentModifier {
	public:
		inline void assign(int x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = clipPixint(x);
			ptr[CHANNELS.A] = 255;
		}

		inline void assign(int r, int g, int b) {
			ptr[CHANNELS.R] = clipPixint(r);
			ptr[CHANNELS.G] = clipPixint(g);
			ptr[CHANNELS.B] = clipPixint(b);
			ptr[CHANNELS.A] = 255;
		}

		inline void assign(int r, int g, int b, int a) {
			ptr[CHANNELS.R] = clipPixint(r);
			ptr[CHANNELS.G] = clipPixint(g);
			ptr[CHANNELS.B] = clipPixint(b);
			ptr[CHANNELS.A] = clipPixint(a);
		}

		inline void assign(float x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = pixfloat2pixbyte(x);
			ptr[CHANNELS.A] = 255;
		}

		inline void assign(float r, float g, float b) {
			ptr[CHANNELS.R] = pixfloat2pixbyte(r);
			ptr[CHANNELS.G] = pixfloat2pixbyte(g);
			ptr[CHANNELS.B] = pixfloat2pixbyte(b);
			ptr[CHANNELS.A] = 255;
		}

		inline void assign(float r, float g, float b, float a) {
			ptr[CHANNELS.R] = pixfloat2pixbyte(r);
			ptr[CHANNELS.G] = pixfloat2pixbyte(g);
			ptr[CHANNELS.B] = pixfloat2pixbyte(b);
			ptr[CHANNELS.A] = pixfloat2pixbyte(a);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			int _a = 255 - P.a;
			ptr[CHANNELS.R] = clipPixint(P.r + ptr[CHANNELS.R] * _a / 255);
			ptr[CHANNELS.G] = clipPixint(P.g + ptr[CHANNELS.G] * _a / 255);
			ptr[CHANNELS.B] = clipPixint(P.b + ptr[CHANNELS.B] * _a / 255);
			ptr[CHANNELS.A] = clipPixint(255 - _a*(255 - ptr[CHANNELS.A]) / 255);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			float _a = (1 - P.a) / 255;
			ptr[CHANNELS.R] = pixfloat2pixbyte(P.r + ptr[CHANNELS.R] * _a);
			ptr[CHANNELS.G] = pixfloat2pixbyte(P.g + ptr[CHANNELS.G] * _a);
			ptr[CHANNELS.B] = pixfloat2pixbyte(P.b + ptr[CHANNELS.B] * _a);
			ptr[CHANNELS.A] = pixfloat2pixbyte(1 - _a * (255 - ptr[CHANNELS.B]) / 255);
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

		QuadByteBitmapWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : QuadByteBitmapReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};


	/**
		Single float bitmap writer
	*/
	class SingleFloatBitmapWriter : public SingleFloatBitmapReader, BitmapContentModifier {
	public:
		inline void assign(int x) {
			*ptr = int2pixfloat(x);
		}

		inline void assign(int r, int g, int b) {
			*ptr = int2pixfloat((r + g + b) / 3);
		}

		inline void assign(int r, int g, int b, int a) {
			*ptr = int2pixfloat((r + g + b) / 3);
		}

		inline void assign(float x) {
			*ptr = clipPixfloat(x);
		}

		inline void assign(float r, float g, float b) {
			*ptr = clipPixfloat((r + g + b) / 3);
		}

		inline void assign(float r, float g, float b, float a) {
			*ptr = clipPixfloat((r + g + b) / 3);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			*ptr = clipPixfloat(((P.r + P.g + P.b) * P.a + *ptr * (255 - P.a) * 3) / 765.0f);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			*ptr = clipPixfloat((P.r + P.g + P.b) * P.a + *ptr * (1 - P.a));
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

		inline void operator=(const pixfloat3& P)  {
			assign(P.r, P.g, P.b);
		}

		inline void operator=(const pixfloat4& P) {
			assign(P.r, P.g, P.b, P.a);
		}

		SingleFloatBitmapWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : SingleFloatBitmapReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};

	
	/**
		Triple float bitmap writer
	*/
	class TripleFloatBitmapWriter : public TripleFloatBitmapReader, BitmapContentModifier {
	public:
		inline void assign(int x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = int2pixfloat(x);
		}

		inline void assign(int r, int g, int b) {
			ptr[CHANNELS.R] = int2pixfloat(r);
			ptr[CHANNELS.G] = int2pixfloat(g);
			ptr[CHANNELS.B] = int2pixfloat(b);
		}

		inline void assign(int r, int g, int b, int a) {
			ptr[CHANNELS.R] = int2pixfloat(r);
			ptr[CHANNELS.G] = int2pixfloat(g);
			ptr[CHANNELS.B] = int2pixfloat(b);
		}

		inline void assign(float x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = clipPixfloat(x);
		}

		inline void assign(float r, float g, float b) {
			ptr[CHANNELS.R] = clipPixfloat(r);
			ptr[CHANNELS.G] = clipPixfloat(g);
			ptr[CHANNELS.B] = clipPixfloat(b);
		}

		inline void assign(float r, float g, float b, float a) {
			ptr[CHANNELS.R] = clipPixfloat(r);
			ptr[CHANNELS.G] = clipPixfloat(g);
			ptr[CHANNELS.B] = clipPixfloat(b);
		}
		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			ptr[CHANNELS.R] = clipPixfloat((P.r * P.a + ptr[CHANNELS.R] * (255 - P.a)) / 65025.0f);
			ptr[CHANNELS.G] = clipPixfloat((P.g * P.a + ptr[CHANNELS.G] * (255 - P.a)) / 65025.0f);
			ptr[CHANNELS.B] = clipPixfloat((P.b * P.a + ptr[CHANNELS.B] * (255 - P.a)) / 65025.0f);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			float _a = 1 - P.a;
			ptr[CHANNELS.R] = clipPixfloat(P.r * P.a + ptr[CHANNELS.R] * _a);
			ptr[CHANNELS.G] = clipPixfloat(P.g * P.a + ptr[CHANNELS.G] * _a);
			ptr[CHANNELS.B] = clipPixfloat(P.b * P.a + ptr[CHANNELS.B] * _a);
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

		TripleFloatBitmapWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : TripleFloatBitmapReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};


	/**
		Quad float bitmap writer
	*/
	class QuadFloatBitmapWriter : public QuadFloatBitmapReader, BitmapContentModifier {
	public:
		inline void assign(int x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = int2pixfloat(x);
			ptr[CHANNELS.A] = 1.0f;
		}

		inline void assign(int r, int g, int b) {
			ptr[CHANNELS.R] = int2pixfloat(r);
			ptr[CHANNELS.G] = int2pixfloat(g);
			ptr[CHANNELS.B] = int2pixfloat(b);
			ptr[CHANNELS.A] = 1.0f;
		}

		inline void assign(int r, int g, int b, int a) {
			ptr[CHANNELS.R] = int2pixfloat(r);
			ptr[CHANNELS.G] = int2pixfloat(g);
			ptr[CHANNELS.B] = int2pixfloat(b);
			ptr[CHANNELS.A] = int2pixfloat(a);
		}

		inline void assign(float x) {
			ptr[CHANNELS.R] = ptr[CHANNELS.G] = ptr[CHANNELS.B] = clipPixfloat(x);
			ptr[CHANNELS.A] = 1.0f;
		}

		inline void assign(float r, float g, float b) {
			ptr[CHANNELS.R] = clipPixfloat(r);
			ptr[CHANNELS.G] = clipPixfloat(g);
			ptr[CHANNELS.B] = clipPixfloat(b);
			ptr[CHANNELS.A] = 1.0f;
		}

		inline void assign(float r, float g, float b, float a) {
			ptr[CHANNELS.R] = clipPixfloat(r);
			ptr[CHANNELS.G] = clipPixfloat(g);
			ptr[CHANNELS.B] = clipPixfloat(b);
			ptr[CHANNELS.A] = clipPixfloat(a);
		}

		inline void operator<<(const pixint1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixint3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixint4& P) {
			int _a = 255 - P.a;
			ptr[CHANNELS.R] = clipPixfloat((P.r + ptr[CHANNELS.R] * _a) / 255.0f);
			ptr[CHANNELS.G] = clipPixfloat((P.g + ptr[CHANNELS.G] * _a) / 255.0f);
			ptr[CHANNELS.B] = clipPixfloat((P.b + ptr[CHANNELS.B] * _a) / 255.0f);
			ptr[CHANNELS.A] = clipPixfloat(1 - _a * ptr[CHANNELS.A] / 255.0f);
		}

		inline void operator<<(const pixfloat1& P) {
			assign(P.x);
		}

		inline void operator<<(const pixfloat3& P) {
			assign(P.r, P.g, P.b);
		}

		inline void operator<<(const pixfloat4& P) {
			float _a = 1 - P.a;
			ptr[CHANNELS.R] = clipPixfloat(P.r + ptr[CHANNELS.R] * _a);
			ptr[CHANNELS.G] = clipPixfloat(P.g + ptr[CHANNELS.G] * _a);
			ptr[CHANNELS.B] = clipPixfloat(P.b + ptr[CHANNELS.B] * _a);
			ptr[CHANNELS.A] = clipPixfloat(1 - _a * (1 - ptr[CHANNELS.A]));
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

		QuadFloatBitmapWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : QuadFloatBitmapReader(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};


	/**
		A generic to access mask bitmap data
	*/
	template<const int num_bits, const int lookup[]> class MaskScanner {
	protected:
		const int pointsPerByte = 8 / num_bits;
		unsigned char
			*data,				//!< all bitmap data
			*ptr,				//!< pointer to current pixel
			bit;				//!< current position bit
		int width, height;		//!< bitmap size in pixels
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

		/**
			Returns 0..MAX_UNNORM_VALUE value at current position
		*/
		inline unsigned char getValue() const {
			return ((*ptr) >> bit) & MAX_UNNORM_VALUE;
		}

		/**
			Returns 0..MAX_UNNORM_VALUE value at (x,y) position
		*/
		inline unsigned char getValue(int x, int y) const {
			msize n = (width*y + x);
			unsigned char
				*p = data + n / pointsPerByte,
				b = (unsigned char)(n % pointsPerByte);
			return ((*p) >> (b*num_bits)) & MAX_UNNORM_VALUE;
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
			if ((bit += num_bits) >= 8) {
				ptr++;
				bit = 0;
			}
		}

		/**
			Move the current position N pixels forward
		*/
		inline void operator+=(const int N)  {
			bit += N*num_bits;
			ptr += bit / 8;
			bit = bit % 8;
		}

		/**
			Changes current position
		*/
		inline void goTo(int x, int y) {
#ifdef BEATMUP_DEBUG
			if (x < 0 || y < 0 || x >= width || y >= height)
				BEATMUP_ERROR("Coordinates outside of image: %d %d (width=%d, height=%d)", x, y, width, height);
#endif
			msize n = (width*y + x);
			ptr = data + n / pointsPerByte,
			bit = (unsigned char)(n % pointsPerByte) * num_bits;			
		}

		MaskScanner(const AbstractBitmap& bitmap, int x = 0, int y = 0) {
#ifdef BEATMUP_DEBUG
			if (bitmap.getBitsPerPixel() != num_bits)
				BEATMUP_ERROR("Invalid mask bitmap scanner");
#endif
			width = bitmap.getWidth();
			height = bitmap.getHeight();
			data = (unsigned char*)bitmap.getData(0, 0);
			goTo(x, y);
		}
	};


	/**
		A generic to write mask bitmap data
	*/
	template<const int num_bits, const int lookup[]> class MaskWriter : public MaskScanner < num_bits, lookup >, BitmapContentModifier {
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

		MaskWriter(AbstractBitmap& bitmap, int x = 0, int y = 0) : MaskScanner < num_bits, lookup >(bitmap, x, y), BitmapContentModifier(bitmap) {}
	};

	//!< lookup tables for masks values
	extern int MASK_LUT_1_BIT[2], MASK_LUT_2_BITS[4], MASK_LUT_4_BITS[16];

	typedef MaskScanner<1, MASK_LUT_1_BIT> BinaryMaskReader;
	typedef MaskScanner<2, MASK_LUT_2_BITS> QuaternaryMaskReader;
	typedef MaskScanner<4, MASK_LUT_4_BITS> HexMaskReader;
	typedef MaskWriter<1, MASK_LUT_1_BIT> BinaryMaskWriter;
	typedef MaskWriter<2, MASK_LUT_2_BITS> QuaternaryMaskWriter;
	typedef MaskWriter<4, MASK_LUT_4_BITS> HexMaskWriter;


	/**
		An exception to throw if a pixel format is not supported
	*/
	class UnsupportedPixelFormat : public Exception {
	public:
		UnsupportedPixelFormat(PixelFormat pf) : Exception("This pixel format is not supported: %s", AbstractBitmap::PIXEL_FORMAT_NAMES[pf]) {};
		static void check(AbstractBitmap& bitmap, PixelFormat pf);
		static void assertMask(AbstractBitmap& bitmap);
		static void assertFloat(AbstractBitmap& bitmap);
		static void assertInt(AbstractBitmap& bitmap);
	};
}