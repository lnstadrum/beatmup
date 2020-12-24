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

#include <Windows.h>
#include <gdiplus.h>
#include "gdi_bitmap.h"
#include "../../gpu/pipeline.h"
#include "../../context.h"
#include "../../exception.h"


using namespace Beatmup;

/**
    A simple wrapper of IL image
*/
class GDIBitmap::Impl {
public:
    Gdiplus::Bitmap* bitmap;
    Gdiplus::Rect size;
    Gdiplus::BitmapData data;


    static void init() {
        static bool gdiStartedUp = false;
        if (!gdiStartedUp) {
            // Start Gdiplus
            Gdiplus::GdiplusStartupInput gdiplusStartupInput;
            ULONG_PTR gdiplusToken;
            Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
        }
    }


    static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
        UINT num = 0;          // number of image encoders
        UINT size = 0;         // size of the image encoder array in bytes

        Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

        Gdiplus::GetImageEncodersSize(&num, &size);
        if (size == 0)
            return -1;  // Failure

        pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
        if (pImageCodecInfo == NULL)
            return -1;  // Failure

        Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

        for (UINT j = 0; j < num; ++j)
        {
            if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
            {
                *pClsid = pImageCodecInfo[j].Clsid;
                free(pImageCodecInfo);
                return j;  // Success
            }
        }

        free(pImageCodecInfo);
        return -1;  // Failure
    }


    inline Impl(Context &ctx, const wchar_t* filename)	{
        init();
        bitmap = new Gdiplus::Bitmap(filename, 1);
        size.X = size.Y = 0;
        size.Width = bitmap->GetWidth();
        size.Height = bitmap->GetHeight();
    }


    inline Impl(Context &ctx, int width, int height, PixelFormat format) {
        init();
        Gdiplus::PixelFormat pf;
        switch (format) {
        case TripleByte:
            pf = PixelFormat24bppRGB;
            break;

        case QuadByte:
            pf = PixelFormat32bppARGB;
            break;

        default:
            throw RuntimeError("Unsupported pixel format");
        }
        bitmap = new Gdiplus::Bitmap(width, height, pf);
        size.X = size.Y = 0;
        size.Width = bitmap->GetWidth();
        size.Height = bitmap->GetHeight();
    }


    inline const PixelFormat getPixelFormat() const {
        switch (bitmap->GetPixelFormat()) {
            case PixelFormat8bppIndexed: return SingleByte;
            case PixelFormat24bppRGB: return TripleByte;
            case PixelFormat32bppARGB: return QuadByte;
        }
        throw RuntimeError("Unsupported pixel format");
    }


    inline const int getWidth() const {
        return size.Width;
    }


    inline const int getHeight() const {
        return size.Height;
    }


    inline int getStride() const {
        return ceili(getBitsPerPixel() * getWidth(), 32) / 8;
    }


    inline void lockPixelData() {
        bitmap->LockBits(&size, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, bitmap->GetPixelFormat(), &data);
    }


    inline void unlockPixelData() {
        bitmap->UnlockBits(&data);
    }


    inline const pixbyte* getData(int x, int y) const {
        return (const pixbyte*)data.Scan0 + x + y * data.Stride;
    }


    inline void save(const WCHAR* filename) {
        CLSID clsid;
        if (wcsstr(filename, L".jpg") || wcsstr(filename, L".jpeg")) {
            if (GetEncoderClsid(L"image/jpeg", &clsid) < 0)
                throw Beatmup::RuntimeError("Unable to get encoder class id");
        }
        else if (wcsstr(filename, L".png")) {
            if (GetEncoderClsid(L"image/png", &clsid) < 0)
                throw Beatmup::RuntimeError("Unable to get encoder class id");
        }
        bitmap->Save(filename, &clsid);
    }
};


GDIBitmap::GDIBitmap(Context &ctx, const wchar_t* filename) :
    AbstractBitmap(ctx)
{
    impl = new Impl(ctx, filename);
}


GDIBitmap::GDIBitmap(Context &ctx, PixelFormat format, int width, int height) : AbstractBitmap(ctx) {
    impl = new Impl(ctx, width, height, format);
}


const PixelFormat GDIBitmap::getPixelFormat() const {
    return impl->getPixelFormat();
}


const int GDIBitmap::getWidth() const {
    return impl->getWidth();
}


const int GDIBitmap::getHeight() const {
    return impl->getHeight();
}


int GDIBitmap::getStride() const {
    return impl->getStride();
}


const msize GDIBitmap::getMemorySize() const {
    return getBitsPerPixel() * getWidth() * getHeight() / 8;
}


void GDIBitmap::lockPixelData() {
    impl->lockPixelData();
}


void GDIBitmap::unlockPixelData() {
    impl->unlockPixelData();
}


const pixbyte* GDIBitmap::getData(int x, int y) const {
    return impl->getData(x, y);
}


void GDIBitmap::save(const wchar_t* filename) {
    impl->save(filename);
}
