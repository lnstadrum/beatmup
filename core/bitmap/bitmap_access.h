/*
    Bitmap accessing utilities.
*/

#pragma once
#include "abstract_bitmap.h"
#include "../exception.h"
#include "pixel_arithmetic.h"

namespace Beatmup {

    /**
     * Color channel order specification
     */
    static const struct {
        const int A, R, G, B;
    } CHANNELS_4 = {
    #ifdef BEATMUP_CHANNEL_ORDER_ARGB
        0, 1, 2, 3
    #elif BEATMUP_CHANNEL_ORDER_BGRA
        3, 2, 1, 0
    #else // rgba
        3, 0, 1, 2
    #endif
    };

    static const struct {
        const int R, G, B;
    } CHANNELS_3 = {
    #ifdef BEATMUP_CHANNEL_ORDER_ARGB
        0, 1, 2
    #elif BEATMUP_CHANNEL_ORDER_BGRA
        2, 1, 0
    #else // rgba
        0, 1, 2
    #endif
    };

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
            DebugAssertion::check(x >= 0 && y >= 0 && x < width && y < height, "Coordinates outside of image: %d %d (width=%d, height=%d)", x, y, width, height);
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
            DebugAssertion::check(
                bitmap.getBitsPerPixel() == 8* num_channels * sizeof(pixel),
                "Invalid bitmap scanner"
            );
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
            return pixint3{ ptr[CHANNELS_3.R], ptr[CHANNELS_3.G], ptr[CHANNELS_3.B] };
        }

        inline pixint3 operator()(int x, int y) const {
            int i = 3 * (y * width + x);
            return pixint3{ data[i + CHANNELS_3.R], data[i + CHANNELS_3.G], data[i + CHANNELS_3.B] };
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
            return pixint4(ptr[CHANNELS_4.R], ptr[CHANNELS_4.G], ptr[CHANNELS_4.B], ptr[CHANNELS_4.A]);
        }

        inline pixint4 operator()(int x, int y) const {
            int i = 4 * (y * width + x);
            return pixint4(data[i + CHANNELS_4.R], data[i + CHANNELS_4.G], data[i + CHANNELS_4.B], data[i + CHANNELS_4.A]);
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
            return pixfloat3{ ptr[CHANNELS_3.R], ptr[CHANNELS_3.G], ptr[CHANNELS_3.B] };
        }

        inline pixfloat3 operator()(int x, int y) const {
            int i = 3 * (y * width + x);
            return pixfloat3{ data[i + CHANNELS_3.R], data[i + CHANNELS_3.G], data[i + CHANNELS_3.B] };
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
            return pixfloat4(ptr[CHANNELS_4.R], ptr[CHANNELS_4.G], ptr[CHANNELS_4.B], ptr[CHANNELS_4.A]);
        }

        inline pixfloat4 operator()(int x, int y) const {
            int i = 4 * (y * width + x);
            return pixfloat4(data[i + CHANNELS_4.R], data[i + CHANNELS_4.G], data[i + CHANNELS_4.B], data[i + CHANNELS_4.A]);
        }

        inline pixfloat4 at(int x, int y) const {
            int i = 4 * (y * width + x);
            return pixfloat4(ptr[i + CHANNELS_4.R], ptr[i + CHANNELS_4.G], ptr[i + CHANNELS_4.B], ptr[i + CHANNELS_4.A]);
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
            ptr[CHANNELS_3.R] = ptr[CHANNELS_3.G] = ptr[CHANNELS_3.B] = clipPixint(x);
        }

        inline void assign(int r, int g, int b) {
            ptr[CHANNELS_3.R] = clipPixint(r);
            ptr[CHANNELS_3.G] = clipPixint(g);
            ptr[CHANNELS_3.B] = clipPixint(b);
        }

        inline void assign(int r, int g, int b, int a) {
            ptr[CHANNELS_3.R] = clipPixint(r);
            ptr[CHANNELS_3.G] = clipPixint(g);
            ptr[CHANNELS_3.B] = clipPixint(b);
        }

        inline void assign(float x) {
            ptr[CHANNELS_3.R] = ptr[CHANNELS_3.G] = ptr[CHANNELS_3.B] = pixfloat2pixbyte(x);
        }

        inline void assign(float r, float g, float b) {
            ptr[CHANNELS_3.R] = pixfloat2pixbyte(r);
            ptr[CHANNELS_3.G] = pixfloat2pixbyte(g);
            ptr[CHANNELS_3.B] = pixfloat2pixbyte(b);
        }

        inline void assign(float r, float g, float b, float a) {
            ptr[CHANNELS_3.R] = pixfloat2pixbyte(r);
            ptr[CHANNELS_3.G] = pixfloat2pixbyte(g);
            ptr[CHANNELS_3.B] = pixfloat2pixbyte(b);
        }

        inline void operator<<(const pixint1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixint3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixint4& P) {
            ptr[CHANNELS_3.R] = clipPixint((P.r * P.a + ptr[CHANNELS_3.R] * (255 - P.a)) / 255);
            ptr[CHANNELS_3.G] = clipPixint((P.g * P.a + ptr[CHANNELS_3.G] * (255 - P.a)) / 255);
            ptr[CHANNELS_3.B] = clipPixint((P.b * P.a + ptr[CHANNELS_3.B] * (255 - P.a)) / 255);
        }

        inline void operator<<(const pixfloat1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixfloat3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixfloat4& P) {
            float _a = (1 - P.a) / 255;
            ptr[CHANNELS_3.R] = pixfloat2pixbyte(P.r * P.a + ptr[CHANNELS_3.R] * _a);
            ptr[CHANNELS_3.G] = pixfloat2pixbyte(P.g * P.a + ptr[CHANNELS_3.G] * _a);
            ptr[CHANNELS_3.B] = pixfloat2pixbyte(P.b * P.a + ptr[CHANNELS_3.B] * _a);
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
            ptr[CHANNELS_4.R] = ptr[CHANNELS_4.G] = ptr[CHANNELS_4.B] = clipPixint(x);
            ptr[CHANNELS_4.A] = 255;
        }

        inline void assign(int r, int g, int b) {
            ptr[CHANNELS_4.R] = clipPixint(r);
            ptr[CHANNELS_4.G] = clipPixint(g);
            ptr[CHANNELS_4.B] = clipPixint(b);
            ptr[CHANNELS_4.A] = 255;
        }

        inline void assign(int r, int g, int b, int a) {
            ptr[CHANNELS_4.R] = clipPixint(r);
            ptr[CHANNELS_4.G] = clipPixint(g);
            ptr[CHANNELS_4.B] = clipPixint(b);
            ptr[CHANNELS_4.A] = clipPixint(a);
        }

        inline void assign(float x) {
            ptr[CHANNELS_4.R] = ptr[CHANNELS_4.G] = ptr[CHANNELS_4.B] = pixfloat2pixbyte(x);
            ptr[CHANNELS_4.A] = 255;
        }

        inline void assign(float r, float g, float b) {
            ptr[CHANNELS_4.R] = pixfloat2pixbyte(r);
            ptr[CHANNELS_4.G] = pixfloat2pixbyte(g);
            ptr[CHANNELS_4.B] = pixfloat2pixbyte(b);
            ptr[CHANNELS_4.A] = 255;
        }

        inline void assign(float r, float g, float b, float a) {
            ptr[CHANNELS_4.R] = pixfloat2pixbyte(r);
            ptr[CHANNELS_4.G] = pixfloat2pixbyte(g);
            ptr[CHANNELS_4.B] = pixfloat2pixbyte(b);
            ptr[CHANNELS_4.A] = pixfloat2pixbyte(a);
        }

        inline void operator<<(const pixint1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixint3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixint4& P) {
            int _a = 255 - P.a;
            ptr[CHANNELS_4.R] = clipPixint(P.r + ptr[CHANNELS_4.R] * _a / 255);
            ptr[CHANNELS_4.G] = clipPixint(P.g + ptr[CHANNELS_4.G] * _a / 255);
            ptr[CHANNELS_4.B] = clipPixint(P.b + ptr[CHANNELS_4.B] * _a / 255);
            ptr[CHANNELS_4.A] = clipPixint(255 - _a*(255 - ptr[CHANNELS_4.A]) / 255);
        }

        inline void operator<<(const pixfloat1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixfloat3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixfloat4& P) {
            float _a = (1 - P.a) / 255;
            ptr[CHANNELS_4.R] = pixfloat2pixbyte(P.r + ptr[CHANNELS_4.R] * _a);
            ptr[CHANNELS_4.G] = pixfloat2pixbyte(P.g + ptr[CHANNELS_4.G] * _a);
            ptr[CHANNELS_4.B] = pixfloat2pixbyte(P.b + ptr[CHANNELS_4.B] * _a);
            ptr[CHANNELS_4.A] = pixfloat2pixbyte(1 - _a * (255 - ptr[CHANNELS_4.B]) / 255);
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
            ptr[CHANNELS_3.R] = ptr[CHANNELS_3.G] = ptr[CHANNELS_3.B] = int2pixfloat(x);
        }

        inline void assign(int r, int g, int b) {
            ptr[CHANNELS_3.R] = int2pixfloat(r);
            ptr[CHANNELS_3.G] = int2pixfloat(g);
            ptr[CHANNELS_3.B] = int2pixfloat(b);
        }

        inline void assign(int r, int g, int b, int a) {
            ptr[CHANNELS_3.R] = int2pixfloat(r);
            ptr[CHANNELS_3.G] = int2pixfloat(g);
            ptr[CHANNELS_3.B] = int2pixfloat(b);
        }

        inline void assign(float x) {
            ptr[CHANNELS_3.R] = ptr[CHANNELS_3.G] = ptr[CHANNELS_3.B] = clipPixfloat(x);
        }

        inline void assign(float r, float g, float b) {
            ptr[CHANNELS_3.R] = clipPixfloat(r);
            ptr[CHANNELS_3.G] = clipPixfloat(g);
            ptr[CHANNELS_3.B] = clipPixfloat(b);
        }

        inline void assign(float r, float g, float b, float a) {
            ptr[CHANNELS_3.R] = clipPixfloat(r);
            ptr[CHANNELS_3.G] = clipPixfloat(g);
            ptr[CHANNELS_3.B] = clipPixfloat(b);
        }
        inline void operator<<(const pixint1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixint3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixint4& P) {
            ptr[CHANNELS_3.R] = clipPixfloat((P.r * P.a + ptr[CHANNELS_3.R] * (255 - P.a)) / 65025.0f);
            ptr[CHANNELS_3.G] = clipPixfloat((P.g * P.a + ptr[CHANNELS_3.G] * (255 - P.a)) / 65025.0f);
            ptr[CHANNELS_3.B] = clipPixfloat((P.b * P.a + ptr[CHANNELS_3.B] * (255 - P.a)) / 65025.0f);
        }

        inline void operator<<(const pixfloat1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixfloat3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixfloat4& P) {
            float _a = 1 - P.a;
            ptr[CHANNELS_3.R] = clipPixfloat(P.r * P.a + ptr[CHANNELS_3.R] * _a);
            ptr[CHANNELS_3.G] = clipPixfloat(P.g * P.a + ptr[CHANNELS_3.G] * _a);
            ptr[CHANNELS_3.B] = clipPixfloat(P.b * P.a + ptr[CHANNELS_3.B] * _a);
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
            ptr[CHANNELS_4.R] = ptr[CHANNELS_4.G] = ptr[CHANNELS_4.B] = int2pixfloat(x);
            ptr[CHANNELS_4.A] = 1.0f;
        }

        inline void assign(int r, int g, int b) {
            ptr[CHANNELS_4.R] = int2pixfloat(r);
            ptr[CHANNELS_4.G] = int2pixfloat(g);
            ptr[CHANNELS_4.B] = int2pixfloat(b);
            ptr[CHANNELS_4.A] = 1.0f;
        }

        inline void assign(int r, int g, int b, int a) {
            ptr[CHANNELS_4.R] = int2pixfloat(r);
            ptr[CHANNELS_4.G] = int2pixfloat(g);
            ptr[CHANNELS_4.B] = int2pixfloat(b);
            ptr[CHANNELS_4.A] = int2pixfloat(a);
        }

        inline void assign(float x) {
            ptr[CHANNELS_4.R] = ptr[CHANNELS_4.G] = ptr[CHANNELS_4.B] = clipPixfloat(x);
            ptr[CHANNELS_4.A] = 1.0f;
        }

        inline void assign(float r, float g, float b) {
            ptr[CHANNELS_4.R] = clipPixfloat(r);
            ptr[CHANNELS_4.G] = clipPixfloat(g);
            ptr[CHANNELS_4.B] = clipPixfloat(b);
            ptr[CHANNELS_4.A] = 1.0f;
        }

        inline void assign(float r, float g, float b, float a) {
            ptr[CHANNELS_4.R] = clipPixfloat(r);
            ptr[CHANNELS_4.G] = clipPixfloat(g);
            ptr[CHANNELS_4.B] = clipPixfloat(b);
            ptr[CHANNELS_4.A] = clipPixfloat(a);
        }

        inline void operator<<(const pixint1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixint3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixint4& P) {
            int _a = 255 - P.a;
            ptr[CHANNELS_4.R] = clipPixfloat((P.r + ptr[CHANNELS_4.R] * _a) / 255.0f);
            ptr[CHANNELS_4.G] = clipPixfloat((P.g + ptr[CHANNELS_4.G] * _a) / 255.0f);
            ptr[CHANNELS_4.B] = clipPixfloat((P.b + ptr[CHANNELS_4.B] * _a) / 255.0f);
            ptr[CHANNELS_4.A] = clipPixfloat(1 - _a * ptr[CHANNELS_4.A] / 255.0f);
        }

        inline void operator<<(const pixfloat1& P) {
            assign(P.x);
        }

        inline void operator<<(const pixfloat3& P) {
            assign(P.r, P.g, P.b);
        }

        inline void operator<<(const pixfloat4& P) {
            float _a = 1 - P.a;
            ptr[CHANNELS_4.R] = clipPixfloat(P.r + ptr[CHANNELS_4.R] * _a);
            ptr[CHANNELS_4.G] = clipPixfloat(P.g + ptr[CHANNELS_4.G] * _a);
            ptr[CHANNELS_4.B] = clipPixfloat(P.b + ptr[CHANNELS_4.B] * _a);
            ptr[CHANNELS_4.A] = clipPixfloat(1 - _a * (1 - ptr[CHANNELS_4.A]));
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
