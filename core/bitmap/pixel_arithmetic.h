/*
    Highly optimized pixel value arithmetic
    Conventions:
        any n-channel color for n<4 is considered as opaque (with alpha = 1 or 255)
*/

#pragma once
#include "abstract_bitmap.h"
#include "../exception.h"
#include <algorithm>
#undef max

#if defined(BEATMUP_CHANNEL_ORDER_BGRA) && defined(BEATMUP_CHANNEL_ORDER_ARGB)
    #error Color order definition conflict
#endif

namespace Beatmup {

    struct pixint1;
    struct pixint3;
    struct pixint4;
    struct pixfloat1;
    struct pixfloat3;
    struct pixfloat4;

    /**
        Monochromatic integer arithmetic
    */
    struct pixint1 {
        typedef int operating_type;
        int x;
        operator pixint3() const;
        operator pixint4() const;
        operator pixfloat1() const;
        operator pixfloat3() const;
        operator pixfloat4() const;
        bool		operator==(const pixint1 P) const;
        void		operator=(const pixint1 P);
        void		operator=(const pixfloat1 P);
        void		operator=(const pixint3 P);
        void		operator=(const pixfloat3 P);
        void		operator=(const pixint4 P);
        void		operator=(const pixfloat4 P);
        pixint1		operator+(const int P) const;
        pixfloat1	operator+(const float P) const;
        pixint1		operator+(const pixint1 P) const;
        pixfloat1	operator+(const pixfloat1 P) const;
        pixint3		operator+(const pixint3 P) const;
        pixfloat3	operator+(const pixfloat3 P) const;
        pixint4		operator+(const pixint4 P) const;
        pixfloat4	operator+(const pixfloat4 P) const;
        pixint1		operator-(const int P) const;
        pixfloat1	operator-(const float P) const;
        pixint1		operator-(const pixint1 P) const;
        pixfloat1	operator-(const pixfloat1 P) const;
        pixint3		operator-(const pixint3 P) const;
        pixfloat3	operator-(const pixfloat3 P) const;
        pixint4		operator-(const pixint4 P) const;
        pixfloat4	operator-(const pixfloat4 P) const;
        pixint1		operator*(const int P) const;
        pixfloat1	operator*(const float P) const;
        pixint1		operator*(const pixint1 P) const;
        pixfloat1	operator*(const pixfloat1 P) const;
        pixint3		operator*(const pixint3 P) const;
        pixfloat3	operator*(const pixfloat3 P) const;
        pixint4		operator*(const pixint4 P) const;
        pixfloat4	operator*(const pixfloat4 P) const;
        pixint1		operator/(const int P) const;
        pixfloat1 makeFloat() const;
        void		zero() { x = 0; }
        int			sum() const { return x; }
        float		mean() const { return (float)x; }
        int			max() const { return x; }
        pixint1		abs() const { return pixint1{ x > 0 ? x : -x }; }
    };

    /**
        Monochromatic floating point arithmetic
    */
    struct pixfloat1 {
        typedef float operating_type;
        pixfloat x;
        operator pixint1() const;
        operator pixint3() const;
        operator pixint4() const;
        operator pixfloat3() const;
        operator pixfloat4() const;
        bool		operator==(const pixfloat1 P) const;
        void		operator=(const pixint1 P);
        void		operator=(const pixfloat1 P);
        void		operator=(const pixint3 P);
        void		operator=(const pixfloat3 P);
        void		operator=(const pixint4 P);
        void		operator=(const pixfloat4 P);
        pixfloat1	operator+(const int P) const;
        pixfloat1	operator+(const float P) const;
        pixfloat1	operator+(const pixint1 P) const;
        pixfloat1	operator+(const pixfloat1 P) const;
        pixfloat3	operator+(const pixint3 P) const;
        pixfloat3	operator+(const pixfloat3 P) const;
        pixfloat4	operator+(const pixint4 P) const;
        pixfloat4	operator+(const pixfloat4 P) const;
        pixfloat1	operator-(const int P) const;
        pixfloat1	operator-(const float P) const;
        pixfloat1	operator-(const pixint1 P) const;
        pixfloat1	operator-(const pixfloat1 P) const;
        pixfloat3	operator-(const pixint3 P) const;
        pixfloat3	operator-(const pixfloat3 P) const;
        pixfloat4	operator-(const pixint4 P) const;
        pixfloat4	operator-(const pixfloat4 P) const;
        pixfloat1	operator*(const int P) const;
        pixfloat1	operator*(const float P) const;
        pixfloat1	operator*(const pixint1 P) const;
        pixfloat1	operator*(const pixfloat1 P) const;
        pixfloat3	operator*(const pixint3 P) const;
        pixfloat3	operator*(const pixfloat3 P) const;
        pixfloat4	operator*(const pixint4 P) const;
        pixfloat4	operator*(const pixfloat4 P) const;
        pixfloat1	operator/(const int P) const;
        pixfloat1 makeFloat() const;
        void		zero() { x = 0; }
        float		sum() const { return x; }
        float		mean() const { return x; }
        float		max() const { return x; }
        pixfloat1	abs() const { return pixfloat1{ x > 0 ? x : -x }; }
    };

    /**
        Trichromatic integer arithmetic
    */
    struct pixint3 {
        typedef int operating_type;
        int r, g, b;
        pixint3(): r(0), g(0), b(0) {}
        pixint3(const color3i& _) : r(_.r), g(_.g), b(_.b) {}
        pixint3(int r, int g, int b) : r(r), g(g), b(b) {}
        operator pixint1() const;
        operator pixint4() const;
        operator pixfloat1() const;
        operator pixfloat3() const;
        operator pixfloat4() const;
        bool		operator==(const pixint3 P) const;
        void		operator=(const pixint1 P);
        void		operator=(const pixfloat1 P);
        void		operator=(const pixint3 P);
        void		operator=(const pixfloat3 P);
        void		operator=(const pixint4 P);
        void		operator=(const pixfloat4 P);
        pixint3		operator+(const int P) const;
        pixfloat3	operator+(const float P) const;
        pixint3		operator+(const pixint1 P) const;
        pixfloat3	operator+(const pixfloat1 P) const;
        pixint3		operator+(const pixint3 P) const;
        pixfloat3	operator+(const pixfloat3 P) const;
        pixint4		operator+(const pixint4 P) const;
        pixfloat4	operator+(const pixfloat4 P) const;
        pixint3		operator-(const int P) const;
        pixfloat3	operator-(const float P) const;
        pixint3		operator-(const pixint1 P) const;
        pixfloat3	operator-(const pixfloat1 P) const;
        pixint3		operator-(const pixint3 P) const;
        pixfloat3	operator-(const pixfloat3 P) const;
        pixint4		operator-(const pixint4 P) const;
        pixfloat4	operator-(const pixfloat4 P) const;
        pixint3		operator*(const int P) const;
        pixfloat3	operator*(const float P) const;
        pixint3		operator*(const pixint1 P) const;
        pixfloat3	operator*(const pixfloat1 P) const;
        pixint3		operator*(const pixint3 P) const;
        pixfloat3	operator*(const pixfloat3 P) const;
        pixint4		operator*(const pixint4 P) const;
        pixfloat4	operator*(const pixfloat4 P) const;
        pixint3		operator/(const int P) const;
        pixfloat3 makeFloat() const;
        void		zero() { r = g = b = 0; }
        int			sum() const { return r + g + b; }
        float		mean() const { return (float)(r + g + b) / 3; }
        int			max() const { return std::max(r, std::max(g, b)); }
        pixint3		abs() const { return pixint3(r > 0 ? r : -r,  g > 0 ? g : -g,  b > 0 ? b : -b); }
    };

    /**
        Trichromatic floating point arithmetic
    */
    struct pixfloat3 {
        typedef float operating_type;
        pixfloat r, g, b;
        pixfloat3(): r(0), g(0), b(0) {}
        pixfloat3(const color3f& _) : r(_.r), g(_.g), b(_.b) {}
        pixfloat3(float r, float g, float b) : r(r), g(g), b(b) {}
        operator pixint1() const;
        operator pixint3() const;
        operator pixint4() const;
        operator pixfloat1() const;
        operator pixfloat4() const;
        bool		operator==(const pixfloat3 P) const;
        void		operator=(const pixint1 P);
        void		operator=(const pixfloat1 P);
        void		operator=(const pixint3 P);
        void		operator=(const pixfloat3 P);
        void		operator=(const pixint4 P);
        void		operator=(const pixfloat4 P);
        pixfloat3	operator+(const int P) const;
        pixfloat3	operator+(const float P) const;
        pixfloat3	operator+(const pixint1 P) const;
        pixfloat3	operator+(const pixfloat1 P) const;
        pixfloat3	operator+(const pixint3 P) const;
        pixfloat3	operator+(const pixfloat3 P) const;
        pixfloat4	operator+(const pixint4 P) const;
        pixfloat4	operator+(const pixfloat4 P) const;
        pixfloat3	operator-(const int P) const;
        pixfloat3	operator-(const float P) const;
        pixfloat3	operator-(const pixint1 P) const;
        pixfloat3	operator-(const pixfloat1 P) const;
        pixfloat3	operator-(const pixint3 P) const;
        pixfloat3	operator-(const pixfloat3 P) const;
        pixfloat4	operator-(const pixint4 P) const;
        pixfloat4	operator-(const pixfloat4 P) const;
        pixfloat3	operator*(const int P) const;
        pixfloat3	operator*(const float P) const;
        pixfloat3	operator*(const pixint1 P) const;
        pixfloat3	operator*(const pixfloat1 P) const;
        pixfloat3	operator*(const pixint3 P) const;
        pixfloat3	operator*(const pixfloat3 P) const;
        pixfloat4	operator*(const pixint4 P) const;
        pixfloat4	operator*(const pixfloat4 P) const;
        pixfloat3	operator/(const int P) const;
        pixfloat3 makeFloat() const;
        void		zero() { r = g = b = 0; }
        float		sum() const { return r + g + b; }
        float		mean() const { return (r + g + b) / 3; }
        float		max() const { return std::max(r, std::max(g, b)); }
        pixfloat3	abs() const { return pixfloat3{ r > 0 ? r : -r,  g > 0 ? g : -g,  b > 0 ? b : -b }; }
    };

    /**
        4-channel integer arithmetic
    */
    struct pixint4 {
        typedef int operating_type;

        union {
            struct {
#ifdef BEATMUP_CHANNEL_ORDER_ARGB
                int a, r, g, b;
#elif BEATMUP_CHANNEL_ORDER_BGRA
                int b, g, r, a;
#else
                int r, g, b, a;
#endif
            };
            int val[4];
        };

        pixint4() : r(0), g(0), b(0), a(0) {}
        pixint4(int r, int g, int b, int a) : r(r), g(g), b(b), a(a) {}
        static pixint4 fromColor(const color4i& _) { return pixint4(_.r, _.g, _.b, _.a); }
        operator pixint1() const;
        operator pixint3() const;
        operator pixfloat1() const;
        operator pixfloat3() const;
        operator pixfloat4() const;
        bool		operator==(const pixint4 P) const;
        void		operator=(const pixint1 P);
        void		operator=(const pixfloat1 P);
        void		operator=(const pixint3 P);
        void		operator=(const pixfloat3 P);
        void		operator=(const pixint4 P);
        void		operator=(const pixfloat4 P);
        pixint4		operator+(const int P) const;
        pixfloat4	operator+(const float P) const;
        pixint4		operator+(const pixint1 P) const;
        pixfloat4	operator+(const pixfloat1 P) const;
        pixint4		operator+(const pixint3 P) const;
        pixfloat4	operator+(const pixfloat3 P) const;
        pixint4		operator+(const pixint4 P) const;
        pixfloat4	operator+(const pixfloat4 P) const;
        pixint4		operator-(const int P) const;
        pixfloat4	operator-(const float P) const;
        pixint4		operator-(const pixint1 P) const;
        pixfloat4	operator-(const pixfloat1 P) const;
        pixint4		operator-(const pixint3 P) const;
        pixfloat4	operator-(const pixfloat3 P) const;
        pixint4		operator-(const pixint4 P) const;
        pixfloat4	operator-(const pixfloat4 P) const;
        pixint4		operator*(const int P) const;
        pixfloat4	operator*(const float P) const;
        pixint4		operator*(const pixint1 P) const;
        pixfloat4	operator*(const pixfloat1 P) const;
        pixint4		operator*(const pixint3 P) const;
        pixfloat4	operator*(const pixfloat3 P) const;
        pixint4		operator*(const pixint4 P) const;
        pixfloat4	operator*(const pixfloat4 P) const;
        pixint4		operator/(const int P) const;
        pixfloat4 makeFloat() const;
        void		zero() { r = g = b = a = 0; }
        int			sum() const { return r + g + b + a; }
        float		mean() const { return (float)(r + g + b + a) / 4; }
        int			max() const { return std::max(std::max(r, g), std::max(b, a)); }
        pixint4		abs() const { return pixint4(r > 0 ? r : -r,  g > 0 ? g : -g,  b > 0 ? b : -b,  a > 0 ? a : -a); }
    };

    /**
        4-channel floating point arithmetic
    */
    struct pixfloat4 {
        typedef float operating_type;

        union {
            struct {
#ifdef BEATMUP_CHANNEL_ORDER_ARGB
                pixfloat a, r, g, b;
#elif BEATMUP_CHANNEL_ORDER_BGRA
                pixfloat b, g, r, a;
#else
                pixfloat r, g, b, a;
#endif
            };
            pixfloat val[4];
        };

        pixfloat4() : r(0), g(0), b(0), a(0) {}
        pixfloat4(const color4f& _) : r(_.r), g(_.g), b(_.b), a(_.a) {}
        pixfloat4(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        pixfloat& operator[](int i) { return val[i]; }
        pixfloat operator[](int i) const { return val[i]; }
        operator pixint1() const;
        operator pixint3() const;
        operator pixint4() const;
        operator pixfloat1() const;
        operator pixfloat3() const;
        operator color4i() const;
        operator color4f() const;
        bool		operator==(const pixfloat4 P) const;
        void		operator=(const pixint1 P);
        void		operator=(const pixfloat1 P);
        void		operator=(const pixint3 P);
        void		operator=(const pixfloat3 P);
        void		operator=(const pixint4 P);
        void		operator=(const pixfloat4 P);
        pixfloat4	operator+(const int P) const;
        pixfloat4	operator+(const float P) const;
        pixfloat4	operator+(const pixint1 P) const;
        pixfloat4	operator+(const pixfloat1 P) const;
        pixfloat4	operator+(const pixint3 P) const;
        pixfloat4	operator+(const pixfloat3 P) const;
        pixfloat4	operator+(const pixint4 P) const;
        pixfloat4	operator+(const pixfloat4 P) const;
        pixfloat4	operator-(const int P) const;
        pixfloat4	operator-(const float P) const;
        pixfloat4	operator-(const pixint1 P) const;
        pixfloat4	operator-(const pixfloat1 P) const;
        pixfloat4	operator-(const pixint3 P) const;
        pixfloat4	operator-(const pixfloat3 P) const;
        pixfloat4	operator-(const pixint4 P) const;
        pixfloat4	operator-(const pixfloat4 P) const;
        pixfloat4	operator*(const int P) const;
        pixfloat4	operator*(const float P) const;
        pixfloat4	operator*(const pixint1 P) const;
        pixfloat4	operator*(const pixfloat1 P) const;
        pixfloat4	operator*(const pixint3 P) const;
        pixfloat4	operator*(const pixfloat3 P) const;
        pixfloat4	operator*(const pixint4 P) const;
        pixfloat4	operator*(const pixfloat4 P) const;
        pixfloat4	operator/(const int P) const;
        pixfloat4 makeFloat() const;
        void		zero() { r = g = b = a = 0; }
        float		sum() const { return r + g + b + a; }
        float		mean() const { return (r + g + b + a) / 4; }
        float		max() const { return std::max(std::max(r, g), std::max(b, a)); }
        pixfloat4	abs() const { return pixfloat4(r > 0 ? r : -r,  g > 0 ? g : -g,  b > 0 ? b : -b, a > 0 ? a : -a); }
    };

    /**
        Converts a floating point pixel value to a 0..255 integer
    */
    inline pixbyte pixfloat2pixbyte(pixfloat x) {
        int i = (int)roundf_fast(x * 255.0f);
        return i > 0 ? (i < 255 ? i : 255) : 0;
    }

    /**
        Converts an integer value to 0..1 floating point pixel value
    */
    inline pixfloat int2pixfloat(int x) {
        return x > 0 ? (x < 255 ? x / 255.0f : 1.0f) : 0.0f;
    }

    /**
        Clips a floating point pixel value to 0..1 range
    */
    inline pixfloat clipPixfloat(pixfloat x) {
        return x > 0.0f ? (x < 1.0f ? x : 1.0f) : 0.0f;
    }

    /**
        Clips an integer pixel value to 0..255 range
    */
    inline pixbyte clipPixint(int x) {
        return x > 0 ? (x < 255 ? x : 255) : 0;
    }

    //////////////////////////////////////////////////////////
    //					Monochromatic integer				//
    //////////////////////////////////////////////////////////

    inline pixint1::operator pixint3() const {
        return pixint3{ x, x, x };
    }

    inline pixint1::operator pixint4() const {
        return pixint4(x, x, x, 255);
    }

    inline pixint1::operator pixfloat1() const {
        return pixfloat1{ x / 255.0f };
    }

    inline pixint1::operator pixfloat3() const {
        float _ = x / 255.0f;
        return pixfloat3{ _, _, _ };
    }

    inline pixint1::operator pixfloat4() const {
        float _ = x / 255.0f;
        return pixfloat4{ _, _, _, 1 };
    }

    inline bool pixint1::operator==(const pixint1 P) const {
        return x == P.x;
    }

    inline void pixint1::operator=(const pixint1 P) {
        x = P.x;
    }

    inline void pixint1::operator=(const pixint3 P) {
        x = (P.r + P.g + P.b) / 3;
    }

    inline void pixint1::operator=(const pixint4 P) {
        x = (P.r + P.g + P.b) / 3;
    }

    inline void pixint1::operator=(const pixfloat1 P) {
        x = (int)roundf_fast(P.x * 255.0f);
    }

    inline void pixint1::operator=(const pixfloat3 P) {
        x = (int)roundf_fast((P.r + P.g + P.b) * 255.0f / 3);
    }

    inline void pixint1::operator=(const pixfloat4 P) {
        x = (int)roundf_fast((P.r + P.g + P.b) * 255.0f / 3);
    }

    // addition
    inline pixint1 pixint1::operator+(const int P) const {
        return pixint1{ x + P };
    }

    inline pixfloat1 pixint1::operator+(const float P) const {
        return pixfloat1{ x / 255.0f + P };
    }

    inline pixint1 pixint1::operator+(const pixint1 P) const {
        return pixint1{ x + P.x };
    }

    inline pixint3 pixint1::operator+(const pixint3 P) const {
        return pixint3{ x + P.r, x + P.g, x + P.b };
    }

    inline pixint4 pixint1::operator+(const pixint4 P) const {
        return pixint4(x + P.r, x + P.g, x + P.b, 255 + P.a);
    }

    inline pixfloat1 pixint1::operator+(const pixfloat1 P) const {
        return pixfloat1{ x / 255.0f + P.x };
    }

    inline pixfloat3 pixint1::operator+(const pixfloat3 P) const {
        return pixfloat3{ x / 255.0f + P.r, x / 255.0f + P.g, x / 255.0f + P.b };
    }

    inline pixfloat4 pixint1::operator+(const pixfloat4 P) const {
        return pixfloat4(x / 255.0f + P.r, x / 255.0f + P.g, x / 255.0f + P.b, 1.0f + P.a);
    }

    // substraction
    inline pixint1 pixint1::operator-(const int P) const {
        return pixint1{ x - P };
    }

    inline pixfloat1 pixint1::operator-(const float P) const {
        return pixfloat1{ x / 255.0f - P };
    }

    inline pixint1 pixint1::operator-(const pixint1 P) const {
        return pixint1{ x - P.x };
    }

    inline pixint3 pixint1::operator-(const pixint3 P) const {
        return pixint3{ x - P.r, x - P.g, x - P.b };
    }

    inline pixint4 pixint1::operator-(const pixint4 P) const {
        return pixint4(x - P.r, x - P.g, x - P.b, 255 - P.a);
    }

    inline pixfloat1 pixint1::operator-(const pixfloat1 P) const {
        return pixfloat1{ x / 255.0f - P.x };
    }

    inline pixfloat3 pixint1::operator-(const pixfloat3 P) const {
        return pixfloat3{ x / 255.0f - P.r, x / 255.0f - P.g, x / 255.0f - P.b };
    }

    inline pixfloat4 pixint1::operator-(const pixfloat4 P) const {
        return pixfloat4(x / 255.0f - P.r, x / 255.0f - P.g, x / 255.0f - P.b, 1.0f - P.a);
    }

    // multiplication
    inline pixint1 pixint1::operator*(const int P) const {
        return pixint1{ x * P };
    }

    inline pixfloat1 pixint1::operator*(const float P) const {
        return pixfloat1{ x * P / 255.0f };
    }

    inline pixint1 pixint1::operator*(const pixint1 P) const {
        return pixint1{ x * P.x };
    }

    inline pixint3 pixint1::operator*(const pixint3 P) const {
        return pixint3{ x * P.r, x * P.g, x * P.b };
    }

    inline pixint4 pixint1::operator*(const pixint4 P) const {
        return pixint4(x * P.r, x * P.g, x * P.b, P.a);
    }

    inline pixfloat1 pixint1::operator*(const pixfloat1 P) const {
        return pixfloat1{ x * P.x / 255.0f };
    }

    inline pixfloat3 pixint1::operator*(const pixfloat3 P) const {
        return pixfloat3{ x * P.r / 255.0f, x * P.g / 255.0f, x * P.b / 255.0f };
    }

    inline pixfloat4 pixint1::operator*(const pixfloat4 P) const {
        return pixfloat4(x * P.r / 255.0f, x * P.g / 255.0f, x * P.b / 255.0f, P.a);
    }

    // division
    inline pixint1 pixint1::operator/(const int P) const {
        return pixint1{ x / P };
    }

    // promotion to pixfloat
    inline pixfloat1 pixint1::makeFloat() const {
        return pixfloat1{ x / 255.0f };
    }

    //////////////////////////////////////////////////////////
    //				Monochromatic floating point			//
    //////////////////////////////////////////////////////////

    inline pixfloat1::operator pixint1() const {
        return pixint1{ roundf_fast(x * 255) };
    }

    inline pixfloat1::operator pixint3() const {
        int _ = roundf_fast(x * 255);
        return pixint3{ _, _, _ };
    }

    inline pixfloat1::operator pixint4() const {
        int _ = roundf_fast(x * 255);
        return pixint4(_, _, _, 255);
    }

    inline pixfloat1::operator pixfloat3() const {
        return pixfloat3{ x, x, x };
    }

    inline pixfloat1::operator pixfloat4() const {
        return pixfloat4{ x, x, x, 1 };
    }

    inline bool pixfloat1::operator==(const pixfloat1 P) const {
        return x == P.x;
    }

    inline void pixfloat1::operator=(const pixint1 P) {
        x = P.x / 255.0f;
    }

    inline void pixfloat1::operator=(const pixfloat1 P) {
        x = P.x;
    }

    inline void pixfloat1::operator=(const pixint3 P) {
        x = (P.r + P.g + P.b) / 765.0f;
    }

    inline void pixfloat1::operator=(const pixfloat3 P) {
        x = (P.r + P.g + P.b) / 3;
    }

    inline void pixfloat1::operator=(const pixint4 P) {
        x = (P.r + P.g + P.b) / 765.0f;
    }

    inline void pixfloat1::operator=(const pixfloat4 P) {
        x = (P.r + P.g + P.b) / 3;
    }

    // addition
    inline pixfloat1 pixfloat1::operator+(const int P) const {
        return pixfloat1{ x + P };
    }

    inline pixfloat1 pixfloat1::operator+(const float P) const {
        return pixfloat1{ x + P };
    }

    inline pixfloat1 pixfloat1::operator+(const pixint1 P) const {
        return pixfloat1{ x + P.x / 255.0f };
    }

    inline pixfloat3 pixfloat1::operator+(const pixint3 P) const {
        return pixfloat3{ x + P.r / 255.0f, x + P.g / 255.0f, x + P.b / 255.0f };
    }

    inline pixfloat4 pixfloat1::operator+(const pixint4 P) const {
        return pixfloat4(x + P.r / 255.0f, x + P.g / 255.0f, x + P.b / 255.0f, 1.0f + P.a / 255.0f);
    }

    inline pixfloat1 pixfloat1::operator+(const pixfloat1 P) const {
        return pixfloat1{ x + P.x };
    }

    inline pixfloat3 pixfloat1::operator+(const pixfloat3 P) const {
        return pixfloat3{ x + P.r, x + P.g, x + P.b };
    }

    inline pixfloat4 pixfloat1::operator+(const pixfloat4 P) const {
        return pixfloat4(x + P.r, x + P.g, x + P.b, 1.0f + P.a);
    }

    // substraction
    inline pixfloat1 pixfloat1::operator-(const int P) const {
        return pixfloat1{ x - P / 255.0f };
    }

    inline pixfloat1 pixfloat1::operator-(const float P) const {
        return pixfloat1{ x - P };
    }

    inline pixfloat1 pixfloat1::operator-(const pixint1 P) const {
        return pixfloat1{ x - P.x / 255.0f };
    }

    inline pixfloat3 pixfloat1::operator-(const pixint3 P) const {
        return pixfloat3{ x - P.r / 255.0f, x - P.g / 255.0f, x - P.b / 255.0f };
    }

    inline pixfloat4 pixfloat1::operator-(const pixint4 P) const {
        return pixfloat4(x - P.r / 255.0f, x - P.g / 255.0f, x - P.b / 255.0f, 1.0f - P.a / 255.0f);
    }

    inline pixfloat1 pixfloat1::operator-(const pixfloat1 P) const {
        return pixfloat1{ x - P.x };
    }

    inline pixfloat3 pixfloat1::operator-(const pixfloat3 P) const {
        return pixfloat3{ x - P.r, x - P.g, x - P.b };
    }

    inline pixfloat4 pixfloat1::operator-(const pixfloat4 P) const {
        return pixfloat4(x - P.r, x - P.g, x - P.b, 1.0f - P.a);
    }

    // multiplication
    inline pixfloat1 pixfloat1::operator*(const int P) const {
        return pixfloat1{ x * P };
    }

    inline pixfloat1 pixfloat1::operator*(const float P) const {
        return pixfloat1{ x * P };
    }

    inline pixfloat1 pixfloat1::operator*(const pixint1 P) const {
        return pixfloat1{ x * P.x / 255.0f };
    }

    inline pixfloat3 pixfloat1::operator*(const pixint3 P) const {
        return pixfloat3{ x * P.r / 255.0f, x * P.g / 255.0f, x * P.b / 255.0f };
    }

    inline pixfloat4 pixfloat1::operator*(const pixint4 P) const {
        return pixfloat4(x * P.r / 255.0f, x * P.g / 255.0f, x * P.b / 255.0f, P.a / 255.0f);
    }

    inline pixfloat1 pixfloat1::operator*(const pixfloat1 P) const {
        return pixfloat1{ x * P.x };
    }

    inline pixfloat3 pixfloat1::operator*(const pixfloat3 P) const {
        return pixfloat3{ x * P.r, x * P.g, x * P.b };
    }

    inline pixfloat4 pixfloat1::operator*(const pixfloat4 P) const {
        return pixfloat4(x * P.r, x * P.g, x * P.b, P.a);
    }

    // division
    inline pixfloat1 pixfloat1::operator/(const int P) const {
        return pixfloat1{ x / P };
    }

    // promotion to pixfloat
    inline pixfloat1 pixfloat1::makeFloat() const {
        return *this;
    }

    //////////////////////////////////////////////////////////
    //					Trichromatic integer				//
    //////////////////////////////////////////////////////////

    inline pixint3::operator pixint1() const {
        return pixint1{(r + g + b) / 3};
    }

    inline pixint3::operator pixint4() const {
        return pixint4(r, g, b, 255);
    }

    inline pixint3::operator pixfloat1() const {
        return pixfloat1{ (r + g + b) / (3 * 255.0f) };
    }

    inline pixint3::operator pixfloat3() const {
        return pixfloat3{ r / 255.0f, g / 255.0f, b / 255.0f };
    }

    inline pixint3::operator pixfloat4() const {
        return pixfloat4{ r / 255.0f, g / 255.0f, b / 255.0f, 1 };
    }

    inline bool pixint3::operator==(const pixint3 P) const {
        return (r == P.r) && (g == P.g) && (b == P.b);
    }

    inline void pixint3::operator=(const pixint1 P) {
        r = g = b = P.x;
    }

    inline void pixint3::operator=(const pixfloat1 P) {
        r = g = b = (int)roundf_fast(P.x * 255.0f);
    }

    inline void pixint3::operator=(const pixint3 P) {
        r = P.r;
        g = P.g;
        b = P.b;
    }

    inline void pixint3::operator=(const pixfloat3 P) {
        r = (int)roundf_fast(P.r * 255.0f);
        g = (int)roundf_fast(P.g * 255.0f);
        b = (int)roundf_fast(P.b * 255.0f);
    }

    inline void pixint3::operator=(const pixint4 P) {
        r = P.r;
        g = P.g;
        b = P.b;
    }

    inline void pixint3::operator=(const pixfloat4 P) {
        r = (int)roundf_fast(P.r * 255.0f);
        g = (int)roundf_fast(P.g * 255.0f);
        b = (int)roundf_fast(P.b * 255.0f);
    }

    // addition
    inline pixint3 pixint3::operator+(const int P) const {
        return pixint3{ r + P, g + P, b + P };
    }

    inline pixfloat3 pixint3::operator+(const float P) const {
        return pixfloat3{ r / 255.0f + P, g / 255.0f + P, b / 255.0f + P };
    }

    inline pixint3 pixint3::operator+(const pixint1 P) const {
        return pixint3{ r + P.x, g + P.x, b + P.x };
    }

    inline pixint3 pixint3::operator+(const pixint3 P) const {
        return pixint3{ r + P.r, g + P.g, b + P.b };
    }

    inline pixint4 pixint3::operator+(const pixint4 P) const {
        return pixint4(r + P.r, g + P.g, b + P.b, 255 + P.a);
    }

    inline pixfloat3 pixint3::operator+(const pixfloat1 P) const {
        return pixfloat3{ r / 255.0f + P.x, g / 255.0f + P.x, b / 255.0f + P.x };
    }

    inline pixfloat3 pixint3::operator+(const pixfloat3 P) const {
        return pixfloat3{ r / 255.0f + P.r, g / 255.0f + P.g, b / 255.0f + P.b };
    }

    inline pixfloat4 pixint3::operator+(const pixfloat4 P) const {
        return pixfloat4(r / 255.0f + P.r, g / 255.0f + P.g, b / 255.0f + P.b, 1.0f + P.a);
    }

    // substraction
    inline pixint3 pixint3::operator-(const int P) const {
        return pixint3{ r - P, g - P, b - P };
    }

    inline pixfloat3 pixint3::operator-(const float P) const {
        return pixfloat3{ r / 255.0f - P, g / 255.0f - P, b / 255.0f - P };
    }

    inline pixint3 pixint3::operator-(const pixint1 P) const {
        return pixint3{ r - P.x, g - P.x, b - P.x };
    }

    inline pixint3 pixint3::operator-(const pixint3 P) const {
        return pixint3{ r - P.r, g - P.g, b - P.b };
    }

    inline pixint4 pixint3::operator-(const pixint4 P) const {
        return pixint4(r - P.r, g - P.g, b - P.b, 255 - P.a);
    }

    inline pixfloat3 pixint3::operator-(const pixfloat1 P) const {
        return pixfloat3{ r / 255.0f - P.x, g / 255.0f - P.x, b / 255.0f - P.x };
    }

    inline pixfloat3 pixint3::operator-(const pixfloat3 P) const {
        return pixfloat3{ r / 255.0f - P.r, g / 255.0f - P.g, b / 255.0f - P.b };
    }

    inline pixfloat4 pixint3::operator-(const pixfloat4 P) const {
        return pixfloat4(r / 255.0f - P.r, g / 255.0f - P.g, b / 255.0f - P.b, 1.0f - P.a);
    }

    // multiplication
    inline pixint3 pixint3::operator*(const int P) const {
        return pixint3{ r * P, g * P, b * P };
    }

    inline pixfloat3 pixint3::operator*(const float P) const {
        return pixfloat3{ r * P / 255.0f, g * P / 255.0f, b * P / 255.0f };
    }

    inline pixint3 pixint3::operator*(const pixint1 P) const {
        return pixint3{ r * P.x / 255, g * P.x / 255, b * P.x / 255 };
    }

    inline pixint3 pixint3::operator*(const pixint3 P) const {
        return pixint3{ r * P.r / 255, g * P.g / 255, b * P.b / 255 };
    }

    inline pixint4 pixint3::operator*(const pixint4 P) const {
        return pixint4(r * P.r / 255, g * P.g / 255, b * P.b / 255, P.a);
    }

    inline pixfloat3 pixint3::operator*(const pixfloat1 P) const {
        return pixfloat3{ r * P.x / 255.0f, g * P.x / 255.0f, b * P.x / 255.0f };
    }

    inline pixfloat3 pixint3::operator*(const pixfloat3 P) const {
        return pixfloat3{ r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f };
    }

    inline pixfloat4 pixint3::operator*(const pixfloat4 P) const {
        return pixfloat4(r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f, P.a);
    }

    // division
    inline pixint3 pixint3::operator/(const int P) const {
        return pixint3{ r / P, g / P, b / P };
    }

    // promotion to pixfloat
    inline pixfloat3 pixint3::makeFloat() const {
        return pixfloat3{ r / 255.0f, g / 255.0f, b / 255.0f };
    }

    //////////////////////////////////////////////////////////
    //				Trichromatic floating point				//
    //////////////////////////////////////////////////////////

    inline pixfloat3::operator pixint1() const {
        return pixint1{ roundf_fast(255 * (r + g + b)) };
    }

    inline pixfloat3::operator pixint3() const {
        return pixint3{ roundf_fast(255 * r), roundf_fast(255 * g), roundf_fast(255 * b) };
    }

    inline pixfloat3::operator pixint4() const {
        return pixint4(roundf_fast(255 * r), roundf_fast(255 * g), roundf_fast(255 * b), 255);
    }

    inline pixfloat3::operator pixfloat1() const {
        return pixfloat1{ (r + g + b) / 3 };
    }

    inline pixfloat3::operator pixfloat4() const {
        return pixfloat4{ r, g, b, 1 };
    }

    inline bool pixfloat3::operator==(const pixfloat3 P) const {
        return (r == P.r) && (g == P.g) && (b == P.b);
    }

    inline void pixfloat3::operator=(const pixint1 P) {
        r = g = b = P.x / 255.0f;
    }

    inline void pixfloat3::operator=(const pixfloat1 P) {
        r = g = b = P.x;
    }

    inline void pixfloat3::operator=(const pixint3 P) {
        r = P.r / 255.0f;
        g = P.g / 255.0f;
        b = P.b / 255.0f;
    }

    inline void pixfloat3::operator=(const pixfloat3 P) {
        r = P.r;
        g = P.g;
        b = P.b;
    }

    inline void pixfloat3::operator=(const pixint4 P) {
        r = P.r / 255.0f;
        g = P.g / 255.0f;
        b = P.b / 255.0f;
    }

    inline void pixfloat3::operator=(const pixfloat4 P) {
        r = P.r;
        g = P.g;
        b = P.b;
    }

    // addition
    inline pixfloat3 pixfloat3::operator+(const int P) const {
        return pixfloat3{ r + P, g + P, b + P };
    }

    inline pixfloat3 pixfloat3::operator+(const float P) const {
        return pixfloat3{ r + P, g + P, b + P };
    }

    inline pixfloat3 pixfloat3::operator+(const pixint1 P) const {
        pixfloat f = P.x / 255.0f;
        return pixfloat3{ r + f, g + f, b + f };
    }

    inline pixfloat3 pixfloat3::operator+(const pixint3 P) const {
        return pixfloat3{ r + P.r / 255.0f, g + P.g / 255.0f, b + P.b / 255.0f };
    }

    inline pixfloat4 pixfloat3::operator+(const pixint4 P) const {
        return pixfloat4(r + P.r / 255.0f, g + P.g / 255.0f, b + P.b / 255.0f, 1.0f + P.a / 255.0f);
    }

    inline pixfloat3 pixfloat3::operator+(const pixfloat1 P) const {
        return pixfloat3{ r + P.x, g + P.x, b + P.x };
    }

    inline pixfloat3 pixfloat3::operator+(const pixfloat3 P) const {
        return pixfloat3{ r + P.r, g + P.g, b + P.b };
    }

    inline pixfloat4 pixfloat3::operator+(const pixfloat4 P) const {
        return pixfloat4(r + P.r, g + P.g, b + P.b, 1.0f + P.a);
    }

    // substraction
    inline pixfloat3 pixfloat3::operator-(const int P) const {
        return pixfloat3{ r - P, g - P, b - P };
    }

    inline pixfloat3 pixfloat3::operator-(const float P) const {
        return pixfloat3{ r - P, g - P, b - P };
    }

    inline pixfloat3 pixfloat3::operator-(const pixint1 P) const {
        pixfloat f = P.x / 255.0f;
        return pixfloat3{ r - f, g - f, b - f };
    }

    inline pixfloat3 pixfloat3::operator-(const pixint3 P) const {
        return pixfloat3{ r - P.r / 255.0f, g - P.g / 255.0f, b - P.b / 255.0f };
    }

    inline pixfloat4 pixfloat3::operator-(const pixint4 P) const {
        return pixfloat4(r - P.r / 255.0f, g - P.g / 255.0f, b - P.b / 255.0f, 1.0f - P.a / 255.0f);
    }

    inline pixfloat3 pixfloat3::operator-(const pixfloat1 P) const {
        return pixfloat3{ r - P.x, g - P.x, b - P.x };
    }

    inline pixfloat3 pixfloat3::operator-(const pixfloat3 P) const {
        return pixfloat3{ r - P.r, g - P.g, b - P.b };
    }

    inline pixfloat4 pixfloat3::operator-(const pixfloat4 P) const {
        return pixfloat4(r - P.r, g - P.g, b - P.b, 1.0f - P.a);
    }

    // multiplication
    inline pixfloat3 pixfloat3::operator*(const int P) const {
        return pixfloat3{ r * P, g * P, b * P };
    }

    inline pixfloat3 pixfloat3::operator*(const float P) const {
        return pixfloat3{ r * P, g * P, b * P };
    }

    inline pixfloat3 pixfloat3::operator*(const pixint1 P) const {
        float f = P.x / 255.0f;
        return pixfloat3{ r * f, g * f, b * f };
    }

    inline pixfloat3 pixfloat3::operator*(const pixint3 P) const {
        return pixfloat3{ r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f };
    }

    inline pixfloat4 pixfloat3::operator*(const pixint4 P) const {
        return pixfloat4(r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f, P.a / 255.0f);
    }

    inline pixfloat3 pixfloat3::operator*(const pixfloat1 P) const {
        return pixfloat3{ r * P.x, g * P.x, b * P.x };
    }

    inline pixfloat3 pixfloat3::operator*(const pixfloat3 P) const {
        return pixfloat3{ r * P.r, g * P.g, b * P.b };
    }

    inline pixfloat4 pixfloat3::operator*(const pixfloat4 P) const {
        return pixfloat4(r * P.r, g * P.g, b * P.b, P.a);
    }

    // division
    inline pixfloat3 pixfloat3::operator/(const int P) const {
        return pixfloat3{ r / P, g / P, b / P };
    }

    // promotion to pixfloat
    inline pixfloat3 pixfloat3::makeFloat() const {
        return *this;
    }

    //////////////////////////////////////////////////////////
    //					4-channel integer					//
    //////////////////////////////////////////////////////////

    inline pixint4::operator pixint1() const {
        return pixint1{ (r + g + b) / 4 };
    }

    inline pixint4::operator pixint3() const {
        return pixint3{ r, g, b };
    }

    inline pixint4::operator pixfloat1() const {
        return pixfloat1{ (r + g + b) / (4 * 255.0f) };
    }

    inline pixint4::operator pixfloat3() const {
        return pixfloat3{ r / 255.0f, g / 255.0f, b / 255.0f };
    }

    inline pixint4::operator pixfloat4() const {
        return pixfloat4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
    }

    inline bool pixint4::operator==(const pixint4 P) const {
        return (r == P.r) && (g == P.g) && (b == P.b) && (a == P.a);
    }

    inline void pixint4::operator=(const pixint1 P) {
        r = g = b = P.x;
        a = 255;
    }

    inline void pixint4::operator=(const pixfloat1 P) {
        r = g = b = (int)roundf_fast(P.x * 255.0f);
        a = 255;
    }

    inline void pixint4::operator=(const pixint3 P) {
        r = P.r;
        g = P.g;
        b = P.b;
        a = 255;
    }

    inline void pixint4::operator=(const pixfloat3 P) {
        r = (int)roundf_fast(P.r * 255.0f);
        g = (int)roundf_fast(P.g * 255.0f);
        b = (int)roundf_fast(P.b * 255.0f);
        a = 255;
    }

    inline void pixint4::operator=(const pixint4 P) {
        r = P.r;
        g = P.g;
        b = P.b;
        a = P.a;
    }

    inline void pixint4::operator=(const pixfloat4 P) {
        r = (int)roundf_fast(P.r * 255.0f);
        g = (int)roundf_fast(P.g * 255.0f);
        b = (int)roundf_fast(P.b * 255.0f);
        a = (int)roundf_fast(P.a * 255.0f);
    }

    // addition
    inline pixint4 pixint4::operator+(const int P) const {
        return pixint4(r + P, g + P, b + P, a + P);
    }

    inline pixfloat4 pixint4::operator+(const float P) const {
        return pixfloat4(r / 255.0f + P, g / 255.0f + P, b / 255.0f + P, a / 255.0f + P);
    }

    inline pixint4 pixint4::operator+(const pixint1 P) const {
        return pixint4(r + P.x, g + P.x, b + P.x, a + 255);
    }

    inline pixint4 pixint4::operator+(const pixint3 P) const {
        return pixint4(r + P.r, g + P.g, b + P.b, a + 255);
    }

    inline pixint4 pixint4::operator+(const pixint4 P) const {
        return pixint4(r + P.r, g + P.g, b + P.b, a + P.a);
    }

    inline pixfloat4 pixint4::operator+(const pixfloat1 P) const {
        return pixfloat4(r / 255.0f + P.x, g / 255.0f + P.x, b / 255.0f + P.x, a / 255.0f + 1.0f);
    }

    inline pixfloat4 pixint4::operator+(const pixfloat3 P) const {
        return pixfloat4(r / 255.0f + P.r, g / 255.0f + P.g, b / 255.0f + P.b, a / 255.0f + 1.0f);
    }

    inline pixfloat4 pixint4::operator+(const pixfloat4 P) const {
        return pixfloat4(r / 255.0f + P.r, g / 255.0f + P.g, b / 255.0f + P.b, a / 255.0f + P.a);
    }

    // substraction
    inline pixint4 pixint4::operator-(const int P) const {
        return pixint4(r - P, g - P, b - P, a - P);
    }

    inline pixfloat4 pixint4::operator-(const float P) const {
        return pixfloat4(r / 255.0f - P, g / 255.0f - P, b / 255.0f - P, a / 255.0f - P);
    }

    inline pixint4 pixint4::operator-(const pixint1 P) const {
        return pixint4(r - P.x, g - P.x, b - P.x, a - 255);
    }

    inline pixint4 pixint4::operator-(const pixint3 P) const {
        return pixint4(r - P.r, g - P.g, b - P.b, a - 255);
    }

    inline pixint4 pixint4::operator-(const pixint4 P) const {
        return pixint4(r - P.r, g - P.g, b - P.b, a - P.a);
    }

    inline pixfloat4 pixint4::operator-(const pixfloat1 P) const {
        return pixfloat4(r / 255.0f - P.x, g / 255.0f - P.x, b / 255.0f - P.x, a / 255.0f - 1.0f);
    }

    inline pixfloat4 pixint4::operator-(const pixfloat3 P) const {
        return pixfloat4(r / 255.0f - P.r, g / 255.0f - P.g, b / 255.0f - P.b, a / 255.0f - 1.0f);
    }

    inline pixfloat4 pixint4::operator-(const pixfloat4 P) const {
        return pixfloat4(r / 255.0f - P.r, g / 255.0f - P.g, b / 255.0f - P.b, a / 255.0f - P.a);
    }

    // multiplication
    inline pixint4 pixint4::operator*(const int P) const {
        return pixint4(r * P, g * P, b * P, a * P);
    }

    inline pixfloat4 pixint4::operator*(const float P) const {
        return pixfloat4(r * P / 255.0f, g * P / 255.0f, b * P / 255.0f, a * P / 255.0f);
    }

    inline pixint4 pixint4::operator*(const pixint1 P) const {
        return pixint4(r * P.x / 255, g * P.x / 255, b * P.x / 255, a);
    }

    inline pixint4 pixint4::operator*(const pixint3 P) const {
        return pixint4(r * P.r / 255, g * P.g / 255, b * P.b / 255, a);
    }

    inline pixint4 pixint4::operator*(const pixint4 P) const {
        return pixint4(r * P.r / 255, g * P.g / 255, b * P.b / 255, a * P.a / 255);
    }

    inline pixfloat4 pixint4::operator*(const pixfloat1 P) const {
        return pixfloat4(r * P.x / 255.0f, g * P.x / 255.0f, b * P.x / 255.0f, a / 255.0f);
    }

    inline pixfloat4 pixint4::operator*(const pixfloat3 P) const {
        return pixfloat4(r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f, a / 255.0f);
    }

    inline pixfloat4 pixint4::operator*(const pixfloat4 P) const {
        return pixfloat4(r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f, a * P.a / 255.0f);
    }

    // division
    inline pixint4 pixint4::operator/(const int P) const {
        return pixint4(r / P, g / P, b / P, a / P);
    }

    // promotion to pixfloat
    inline pixfloat4 pixint4::makeFloat() const {
        return pixfloat4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    //////////////////////////////////////////////////////////
    //				4-channel floating point				//
    //////////////////////////////////////////////////////////

    inline pixfloat4::operator pixint1() const {
        return pixint1{ roundf_fast(85 * (r + g + b)) };
    }

    inline pixfloat4::operator pixint3() const {
        return pixint3{ roundf_fast(255 * r), roundf_fast(255 * g), roundf_fast(255 * b) };
    }

    inline pixfloat4::operator pixint4() const {
        return pixint4(roundf_fast(255 * r), roundf_fast(255 * g), roundf_fast(255 * b), roundf_fast(255 * a));
    }
    inline pixfloat4::operator pixfloat1() const {
        return pixfloat1{ (r + g + b) / 3 };
    }

    inline pixfloat4::operator pixfloat3() const {
        return pixfloat3{ r, g, b };
    }

    inline pixfloat4::operator color4f() const {
        return color4f{ r, g, b, a };
    }

    inline pixfloat4::operator color4i() const {
        return color4i{ pixfloat2pixbyte(r), pixfloat2pixbyte(g), pixfloat2pixbyte(b), pixfloat2pixbyte(a) };
    }

    inline bool pixfloat4::operator==(const pixfloat4 P) const {
        return (r == P.r) && (g == P.g) && (b == P.b) && (a == P.a);
    }

    inline void pixfloat4::operator=(const pixint1 P) {
        r = g = b = P.x / 255.0f;
        a = 1.0f;
    }

    inline void pixfloat4::operator=(const pixfloat1 P) {
        r = g = b = P.x;
        a = 1.0f;
    }

    inline void pixfloat4::operator=(const pixint3 P) {
        r = P.r / 255.0f;
        g = P.g / 255.0f;
        b = P.b / 255.0f;
        a = 1.0f;
    }

    inline void pixfloat4::operator=(const pixfloat3 P) {
        r = P.r;
        g = P.g;
        b = P.b;
        a = 1.0f;
    }

    inline void pixfloat4::operator=(const pixint4 P) {
        r = P.r / 255.0f;
        g = P.g / 255.0f;
        b = P.b / 255.0f;
        a = P.a / 255.0f;
    }

    inline void pixfloat4::operator=(const pixfloat4 P) {
        r = P.r;
        g = P.g;
        b = P.b;
        a = P.a;
    }

    // addition
    inline pixfloat4 pixfloat4::operator+(const int P) const {
        return pixfloat4(r + P, g + P, b + P, a + P);
    }

    inline pixfloat4 pixfloat4::operator+(const float P) const {
        return pixfloat4(r + P, g + P, b + P, a + P);
    }

    inline pixfloat4 pixfloat4::operator+(const pixint1 P) const {
        pixfloat f = P.x / 255.0f;
        return pixfloat4(r + f, g + f, b + f, a + 1.0f);
    }

    inline pixfloat4 pixfloat4::operator+(const pixint3 P) const {
        return pixfloat4(r + P.r / 255.0f, g + P.g / 255.0f, b + P.b / 255.0f, a + 1.0f);
    }

    inline pixfloat4 pixfloat4::operator+(const pixint4 P) const {
        return pixfloat4(r + P.r / 255.0f, g + P.g / 255.0f, b + P.b / 255.0f, a + P.a / 255.0f);
    }

    inline pixfloat4 pixfloat4::operator+(const pixfloat1 P) const {
        return pixfloat4(r + P.x, g + P.x, b + P.x, a + 1.0f);
    }

    inline pixfloat4 pixfloat4::operator+(const pixfloat3 P) const {
        return pixfloat4(r + P.r, g + P.g, b + P.b, a + 1.0f);
    }

    inline pixfloat4 pixfloat4::operator+(const pixfloat4 P) const {
        return pixfloat4(r + P.r, g + P.g, b + P.b, a + P.a);
    }

    // substraction
    inline pixfloat4 pixfloat4::operator-(const int P) const {
        return pixfloat4(r - P, g - P, b - P, a - P);
    }

    inline pixfloat4 pixfloat4::operator-(const float P) const {
        return pixfloat4(r - P, g - P, b - P, a - P);
    }

    inline pixfloat4 pixfloat4::operator-(const pixint1 P) const {
        pixfloat f = P.x / 255.0f;
        return pixfloat4(r - f, g - f, b - f, a - 1.0f);
    }

    inline pixfloat4 pixfloat4::operator-(const pixint3 P) const {
        return pixfloat4(r - P.r / 255.0f, g - P.g / 255.0f, b - P.b / 255.0f, a - 1.0f);
    }

    inline pixfloat4 pixfloat4::operator-(const pixint4 P) const {
        return pixfloat4(r - P.r / 255.0f, g - P.g / 255.0f, b - P.b / 255.0f, a - P.a / 255.0f - 1.0f);
    }

    inline pixfloat4 pixfloat4::operator-(const pixfloat1 P) const {
        return pixfloat4(r - P.x, g - P.x, b - P.x, a - 1.0f);
    }

    inline pixfloat4 pixfloat4::operator-(const pixfloat3 P) const {
        return pixfloat4(r - P.r, g - P.g, b - P.b, a - 1.0f);
    }

    inline pixfloat4 pixfloat4::operator-(const pixfloat4 P) const {
        return pixfloat4(r - P.r, g - P.g, b - P.b, a - P.a);
    }

    // multiplication
    inline pixfloat4 pixfloat4::operator*(const int P) const {
        return pixfloat4(r * P, g * P, b * P, a * P);
    }

    inline pixfloat4 pixfloat4::operator*(const float P) const {
        return pixfloat4(r * P, g * P, b * P, a * P);
    }

    inline pixfloat4 pixfloat4::operator*(const pixint1 P) const {
        float f = P.x / 255.0f;
        return pixfloat4(r * f, g * f, b * f, a);
    }

    inline pixfloat4 pixfloat4::operator*(const pixint3 P) const {
        return pixfloat4(r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f, a);
    }

    inline pixfloat4 pixfloat4::operator*(const pixint4 P) const {
        return pixfloat4(r * P.r / 255.0f, g * P.g / 255.0f, b * P.b / 255.0f, a * P.a / 255.0f);
    }

    inline pixfloat4 pixfloat4::operator*(const pixfloat1 P) const {
        return pixfloat4(r * P.x, g * P.x, b * P.x, a);
    }

    inline pixfloat4 pixfloat4::operator*(const pixfloat3 P) const {
        return pixfloat4(r * P.r, g * P.g, b * P.b, a);
    }

    inline pixfloat4 pixfloat4::operator*(const pixfloat4 P) const {
        return pixfloat4(r * P.r, g * P.g, b * P.b, a * P.a);
    }

    // division
    inline pixfloat4 pixfloat4::operator/(const int P) const {
        return pixfloat4(r / P, g / P, b / P, a / P);
    }

    // promotion to pixfloat
    inline pixfloat4 pixfloat4::makeFloat() const {
        return *this;
    }

    //////////////////////////////////////////////////////////
    //					GLOBAL OPERATORS					//
    //////////////////////////////////////////////////////////

    inline pixint1 operator-(int x, pixint1 P) {
        return pixint1{ x - P.x };
    }
}
