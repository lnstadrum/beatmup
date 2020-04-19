/*
    Bitmap interpolation utilities
*/
#pragma once
#include "bitmap_access.h"

namespace Beatmup {

    /**
        Nearest neighbour bitmap interpolation
    */
    template<class scanner, typename pixel> class NearestNeighborInterpolator : public scanner {
    public:
        NearestNeighborInterpolator(const AbstractBitmap& bitmap, int x = 0, int y = 0) : scanner(bitmap, x, y) {}
        inline pixel operator()() const {
            return scanner::operator()();
        }

        inline pixel operator()(float x, float y) const {
            return scanner::operator() (roundf_fast(x), roundf_fast(y));
        }
    };

    typedef NearestNeighborInterpolator < SingleByteBitmapReader, pixint1 > SingleByteNearestNeighborInterpolator;
    typedef NearestNeighborInterpolator < TripleByteBitmapReader, pixint3 > TripleByteNearestNeighborInterpolator;
    typedef NearestNeighborInterpolator < QuadByteBitmapReader, pixint4 > QuadByteNearestNeighborInterpolator;
    typedef NearestNeighborInterpolator < SingleFloatBitmapReader, pixfloat1 > SingleFloatNearestNeighborInterpolator;
    typedef NearestNeighborInterpolator < TripleFloatBitmapReader, pixfloat3 > TripleFloatNearestNeighborInterpolator;
    typedef NearestNeighborInterpolator < QuadFloatBitmapReader, pixfloat4 > QuadFloatNearestNeighborInterpolator;

    /**
        Single byte bitmap bilinear interpolation
    */
    class SingleByteBilinearInterpolator : public SingleByteBitmapReader {
    public:
        SingleByteBilinearInterpolator(const AbstractBitmap& bitmap, int x = 0, int y = 0) : SingleByteBitmapReader(bitmap, x, y) {}

        inline pixint1 operator()() const {
            return SingleByteBitmapReader::operator()();
        }

        inline pixint1 operator()(float x, float y) const {
            int ix = (int)x, iy = (int)y;
            pixbyte* p = jump(ix, iy);
            if (ix < width-1) {
                float fx = x - ix;
                if (iy < height-1) {
                    float fy = y - iy;
                    return pixint1{ roundf_fast((p[0] * (1 - fx) + p[1] * fx) * (1 - fy) + (p[width] * (1 - fx) + p[width + 1] * fx) * fy) };
                } else
                    return pixint1{ roundf_fast(p[0] * (1 - fx) + p[1] * fx) };
            }
            else
                if (iy < height-1) {
                    float fy = y - iy;
                    return pixint1{ roundf_fast(p[0] * (1 - fy) + p[width] * fy) };
                }
                else
                    return pixint1{ p[0] };
        }
    };

    /**
        Triple byte bitmap bilinear interpolation
    */
    class TripleByteBilinearInterpolator : public TripleByteBitmapReader {
    public:
        TripleByteBilinearInterpolator(const AbstractBitmap& bitmap, int x = 0, int y = 0) : TripleByteBitmapReader(bitmap, x, y) {}

        inline pixint3 operator()() const {
            return TripleByteBitmapReader::operator()();
        }

        inline pixint3 operator()(float x, float y) const {
            int
                ix = (int)x, iy = (int)y,
                i1 = 3 * width;
            pixbyte* p = jump(ix, iy);
            if (ix < width-1) {
                float fx = x - ix;
                if (iy < height-1) {
                    float
                        fy = y - iy,
                        w0 = (1 - fx) * (1 - fy),
                        w1 = fx * (1 - fy),
                        w2 = (1 - fx) * fy,
                        w3 = fx*fy;
                    return pixint3 {
                        roundf_fast(p[0] * w0 + p[3] * w1 + p[i1  ] * w2 + p[i1+3] * w3),
                        roundf_fast(p[1] * w0 + p[4] * w1 + p[i1+1] * w2 + p[i1+4] * w3),
                        roundf_fast(p[2] * w0 + p[5] * w1 + p[i1+2] * w2 + p[i1+5] * w3)
                    };
                }
                else
                    return pixint3{
                        roundf_fast(p[0] * (1 - fx) + p[3] * fx),
                        roundf_fast(p[1] * (1 - fx) + p[4] * fx),
                        roundf_fast(p[2] * (1 - fx) + p[5] * fx)
                    };
            }
            else
                if (iy < height-1) {
                    float fy = y - iy;
                    return pixint3{
                        roundf_fast(p[0] * (1 - fy) + p[i1  ] * fy),
                        roundf_fast(p[1] * (1 - fy) + p[i1+1] * fy),
                        roundf_fast(p[2] * (1 - fy) + p[i1+2] * fy)
                    };
                }
                else
                    return pixint3{ p[0], p[1], p[2] };
        }
    };

    /**
        Quad byte bitmap bilinear interpolation
    */
    class QuadByteBilinearInterpolator : public QuadByteBitmapReader {
    public:
        QuadByteBilinearInterpolator(const AbstractBitmap& bitmap, int x = 0, int y = 0) : QuadByteBitmapReader(bitmap, x, y) {}

        inline pixint4 operator()() const {
            return QuadByteBitmapReader::operator()();
        }

        inline pixint4 operator()(float x, float y) const {
            int
                ix = (int)x, iy = (int)y,
                i1 = 4 * width;
            pixbyte* p = jump(ix, iy);
            if (ix < width-1) {
                float fx = x - ix;
                if (iy < height-1) {
                    float
                        fy = y - iy,
                        w0 = (1 - fx) * (1 - fy),
                        w1 = fx * (1 - fy),
                        w2 = (1 - fx) * fy,
                        w3 = fx*fy;
                    return pixint4 {
                        roundf_fast(p[0] * w0 + p[4] * w1 + p[i1    ] * w2 + p[i1 + 4] * w3),
                        roundf_fast(p[1] * w0 + p[5] * w1 + p[i1 + 1] * w2 + p[i1 + 5] * w3),
                        roundf_fast(p[2] * w0 + p[6] * w1 + p[i1 + 2] * w2 + p[i1 + 6] * w3),
                        roundf_fast(p[3] * w0 + p[7] * w1 + p[i1 + 3] * w2 + p[i1 + 7] * w3)
                    };
                }
                else {
                    float w = 1 - fx;
                    return pixint4 {
                        roundf_fast(p[0] * w + p[4] * fx),
                        roundf_fast(p[1] * w + p[5] * fx),
                        roundf_fast(p[2] * w + p[6] * fx),
                        roundf_fast(p[3] * w + p[7] * fx)
                    };
                }
            }
            else
                if (iy < height-1) {
                    float fy = y - iy, w = 1 - fy;
                    return pixint4 {
                        roundf_fast(p[0] * w + p[i1    ] * fy),
                        roundf_fast(p[1] * w + p[i1 + 1] * fy),
                        roundf_fast(p[2] * w + p[i1 + 2] * fy),
                        roundf_fast(p[3] * w + p[i1 + 3] * fy)
                    };
                }
                else
                    return pixint4{ p[0], p[1], p[2], p[3] };
        }
    };

    /**
        Floating point bitmap bilinear interpolation, implemented using pixel arithmetics
    */
    template<class scanner, typename pixel> class FloatBilinearInterpolator : public scanner {
    public:
        FloatBilinearInterpolator(const AbstractBitmap& bitmap, int x = 0, int y = 0) : scanner(bitmap, x, y) {}

        inline pixel operator()() const {
            return scanner::operator()();
        }

        inline pixel operator()(float x, float y) const {
            int ix = (int)x, iy = (int)y;
            pixel* p = (pixel*)scanner::jump(ix, iy);
            if (ix < scanner::width-1) {
                float fx = x - ix;
                if (iy < scanner::height-1) {
                    float fy = y - iy;
                    return (p[0] * (1 - fx) + p[1] * fx) * (1 - fy) + (p[scanner::width] * (1 - fx) + p[scanner::width + 1] * fx) * fy;
                }
                else
                    return p[0] * (1 - fx) + p[1] * fx;
            }
            else
                if (iy < scanner::height-1) {
                    float fy = y - iy;
                    return p[0] * (1 - fy) + p[scanner::width] * fy;
                }
                else
                    return p[0];
        }
    };

    typedef FloatBilinearInterpolator < SingleFloatBitmapReader, pixfloat1 > SingleFloatBilinearInterpolator;
    typedef FloatBilinearInterpolator < TripleFloatBitmapReader, pixfloat3 > TripleFloatBilinearInterpolator;
    typedef FloatBilinearInterpolator < QuadFloatBitmapReader, pixfloat4 > QuadFloatBilinearInterpolator;
}
