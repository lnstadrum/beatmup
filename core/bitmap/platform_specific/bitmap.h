/**
	Defines a platform-specific bitmap
*/

#ifdef BEATMUP_PLATFORM_WINDOWS
#include "gdi_bitmap.h"
namespace Beatmup {
	typedef GDIBitmap Bitmap;
}
#else
	#error No platform-specific bitmap is defined for the current platform
#endif