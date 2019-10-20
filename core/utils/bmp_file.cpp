#include "bmp_file.h"
#include "../bitmap/bitmap_access.h"
#include "../exception.h"

using namespace Beatmup;

static const uint8_t
	// swapping bit order in a byte (most significant <-> least significant)
	LSB_MSB_REVERSE_LOOKUP[] = {
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
	},

	// swapping least and most significant nibbles (half-byte) in a byte
	LSN_MSN_REVERSE_LOOKUP[] = {
		0x0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
		0x1, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x81, 0x91, 0xa1, 0xb1, 0xc1, 0xd1, 0xe1, 0xf1,
		0x2, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, 0x82, 0x92, 0xa2, 0xb2, 0xc2, 0xd2, 0xe2, 0xf2,
		0x3, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73, 0x83, 0x93, 0xa3, 0xb3, 0xc3, 0xd3, 0xe3, 0xf3,
		0x4, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 0x84, 0x94, 0xa4, 0xb4, 0xc4, 0xd4, 0xe4, 0xf4,
		0x5, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75, 0x85, 0x95, 0xa5, 0xb5, 0xc5, 0xd5, 0xe5, 0xf5,
		0x6, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76, 0x86, 0x96, 0xa6, 0xb6, 0xc6, 0xd6, 0xe6, 0xf6,
		0x7, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77, 0x87, 0x97, 0xa7, 0xb7, 0xc7, 0xd7, 0xe7, 0xf7,
		0x8, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78, 0x88, 0x98, 0xa8, 0xb8, 0xc8, 0xd8, 0xe8, 0xf8,
		0x9, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x79, 0x89, 0x99, 0xa9, 0xb9, 0xc9, 0xd9, 0xe9, 0xf9,
		0xa, 0x1a, 0x2a, 0x3a, 0x4a, 0x5a, 0x6a, 0x7a, 0x8a, 0x9a, 0xaa, 0xba, 0xca, 0xda, 0xea, 0xfa,
		0xb, 0x1b, 0x2b, 0x3b, 0x4b, 0x5b, 0x6b, 0x7b, 0x8b, 0x9b, 0xab, 0xbb, 0xcb, 0xdb, 0xeb, 0xfb,
		0xc, 0x1c, 0x2c, 0x3c, 0x4c, 0x5c, 0x6c, 0x7c, 0x8c, 0x9c, 0xac, 0xbc, 0xcc, 0xdc, 0xec, 0xfc,
		0xd, 0x1d, 0x2d, 0x3d, 0x4d, 0x5d, 0x6d, 0x7d, 0x8d, 0x9d, 0xad, 0xbd, 0xcd, 0xdd, 0xed, 0xfd,
		0xe, 0x1e, 0x2e, 0x3e, 0x4e, 0x5e, 0x6e, 0x7e, 0x8e, 0x9e, 0xae, 0xbe, 0xce, 0xde, 0xee, 0xfe,
		0xf, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f, 0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff
	};


const BmpFile::Header BmpFile::BMP_HEADER_REFERENCE = {
	{ 'B', 'M' },		// magic
	0, { 0, 0 }, 0,
	40,					// headerSize
	0, 0,
	1,					// numColorPlanes
	0,
	0,					// compression
	0, 0, 0, 0
};


static const uint32_t	BMP_32BIT_COMPRESSION = 3;	// "compression" header value for 32 bits bitmaps


BmpFile::BmpFile(const char* filename) {
	// open file
	in.open(filename, std::fstream::in | std::fstream::binary);
	if (!in.good())
		throw IOError(filename, "Cannot read.");

	// grab header & check some fields
	in.read((char*)&header, sizeof(header));
	if (header.magic[0] != BMP_HEADER_REFERENCE.magic[0]
		|| header.magic[1] != BMP_HEADER_REFERENCE.magic[1])
		throw IOError(filename, "Likely not a bmp file. Bad magic.");
	if (header.headerSize < BMP_HEADER_REFERENCE.headerSize
		|| header.numColorPlanes != BMP_HEADER_REFERENCE.numColorPlanes)
		throw IOError(filename, "Likely not a bmp file. Invalid header.");
	if (header.bpp == 32 && header.compression != BMP_32BIT_COMPRESSION)
		throw IOError(filename, "Unsupported 32-bit bitmap compression.");
	else if (header.bpp != 32 && header.compression != BMP_HEADER_REFERENCE.compression)
		throw IOError(filename, "Compressed bitmaps are not supported.");
}


void BmpFile::load(void* pixels, const uint32_t pixelsSizeInBytes) {
	const msize
		rowSize = ceili(header.width * header.bpp, 8),
		rowAlign = ceili(rowSize, 4) * 4 - rowSize;
	RuntimeError::check(pixelsSizeInBytes >= rowSize * header.height,
		"Cannot fit BMP file to a pixel buffer");
	char pad[3];
	in.seekg(header.offset, std::ios_base::beg);
	for (int y = header.height - 1; y >= 0 && !in.eof(); --y) {
		char* ptr = (char*)pixels + y * rowSize;
		if (header.bpp == 32)
			for (int x = 0; x < header.width && !in.eof(); ++x, ptr+=4) {
				in.read(ptr + CHANNELS_4.A, 1);
				in.read(ptr + CHANNELS_4.B, 1);
				in.read(ptr + CHANNELS_4.G, 1);
				in.read(ptr + CHANNELS_4.R, 1);
			}
		else if (header.bpp == 24)
			for (int x = 0; x < header.width && !in.eof(); ++x, ptr+=3) {
				in.read(ptr + CHANNELS_3.B, 1);
				in.read(ptr + CHANNELS_3.G, 1);
				in.read(ptr + CHANNELS_3.R, 1);
			}
		else if (header.bpp == 4)
			for (msize i = 0; i < rowSize && !in.eof(); ++i, ptr++) {
				in.read(ptr, 1);
				*ptr = (char)LSN_MSN_REVERSE_LOOKUP[(uint8_t)*ptr];
			}
		else if (header.bpp == 1)
			for (msize i = 0; i < rowSize && !in.eof(); ++i, ptr++) {
				in.read(ptr, 1);
				*ptr = (char)LSB_MSB_REVERSE_LOOKUP[(uint8_t)*ptr];
			}
		else {
			in.read(ptr, rowSize);
			ptr += rowSize;
		}
		in.read(pad, rowAlign);
	}
}


void BmpFile::save(
	const void* pixels,
	int32_t width,
	int32_t height,
	uint8_t bpp,
	const char* filename
) {
	// setup header
	Header header = BMP_HEADER_REFERENCE;
	switch (bpp) {
		case 1:
		case 4:
		case 24:
			header.bpp = bpp;
		break;

		case 8:
			header.bpp = 8;
			header.numImportantColors = 256;
		break;

		case 32:
			header.bpp = 32;
			header.compression = BMP_32BIT_COMPRESSION;
		break;

		default:
			throw IOError(filename, "Unsupported number of bits per pixel when saving a BMP file.");
	}
	const msize
		rowSize = ceili(width * bpp, 8),
		rowAlign = ceili(rowSize, 4) * 4 - rowSize;
	header.width = width;
	header.height = height;
	header.offset = sizeof(header);
	header.size = ceili(rowSize, 4) * 4 * height;    // offset added later

	// setup output stream
	std::fstream out(filename, std::ios::out | std::ios::binary);
	if (!out.good())
		throw IOError(filename, "Cannot write to file.");

	static const uint32_t
		BMP_1BIT_COLOR_SPACE_HEADER[] = { 2 },
		BMP_4BIT_COLOR_SPACE_HEADER[] = { 16 },
		BMP_8BIT_COLOR_SPACE_HEADER[] = { 256 },
		BMP_24BIT_COLOR_SPACE_HEADER[] = { 0 },
		BMP_32BIT_COLOR_SPACE_HEADER[] = { 0, 0xff000000, 0xff0000, 0xff00, 0xff },
		BMP_COLOR_SPACE_INFO[] = { 0x73524742, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2, 0, 0, 0 },
		BMP_8BIT_COLOR_TABLE[] = { 0, 0x10101, 0x20202, 0x30303, 0x40404, 0x50505, 0x60606, 0x70707, 0x80808, 0x90909, 0xa0a0a, 0xb0b0b, 0xc0c0c, 0xd0d0d, 0xe0e0e, 0xf0f0f, 0x101010, 0x111111, 0x121212, 0x131313, 0x141414, 0x151515, 0x161616, 0x171717, 0x181818, 0x191919, 0x1a1a1a, 0x1b1b1b, 0x1c1c1c, 0x1d1d1d, 0x1e1e1e, 0x1f1f1f, 0x202020, 0x212121, 0x222222, 0x232323, 0x242424, 0x252525, 0x262626, 0x272727, 0x282828, 0x292929, 0x2a2a2a, 0x2b2b2b, 0x2c2c2c, 0x2d2d2d, 0x2e2e2e, 0x2f2f2f, 0x303030, 0x313131, 0x323232, 0x333333, 0x343434, 0x353535, 0x363636, 0x373737, 0x383838, 0x393939, 0x3a3a3a, 0x3b3b3b, 0x3c3c3c, 0x3d3d3d, 0x3e3e3e, 0x3f3f3f, 0x404040, 0x414141, 0x424242, 0x434343, 0x444444, 0x454545, 0x464646, 0x474747, 0x484848, 0x494949, 0x4a4a4a, 0x4b4b4b, 0x4c4c4c, 0x4d4d4d, 0x4e4e4e, 0x4f4f4f, 0x505050, 0x515151, 0x525252, 0x535353, 0x545454, 0x555555, 0x565656, 0x575757, 0x585858, 0x595959, 0x5a5a5a, 0x5b5b5b, 0x5c5c5c, 0x5d5d5d, 0x5e5e5e, 0x5f5f5f, 0x606060, 0x616161, 0x626262, 0x636363, 0x646464, 0x656565, 0x666666, 0x676767, 0x686868, 0x696969, 0x6a6a6a, 0x6b6b6b, 0x6c6c6c, 0x6d6d6d, 0x6e6e6e, 0x6f6f6f, 0x707070, 0x717171, 0x727272, 0x737373, 0x747474, 0x757575, 0x767676, 0x777777, 0x787878, 0x797979, 0x7a7a7a, 0x7b7b7b, 0x7c7c7c, 0x7d7d7d, 0x7e7e7e, 0x7f7f7f, 0x808080, 0x818181, 0x828282, 0x838383, 0x848484, 0x858585, 0x868686, 0x878787, 0x888888, 0x898989, 0x8a8a8a, 0x8b8b8b, 0x8c8c8c, 0x8d8d8d, 0x8e8e8e, 0x8f8f8f, 0x909090, 0x919191, 0x929292, 0x939393, 0x949494, 0x959595, 0x969696, 0x979797, 0x989898, 0x999999, 0x9a9a9a, 0x9b9b9b, 0x9c9c9c, 0x9d9d9d, 0x9e9e9e, 0x9f9f9f, 0xa0a0a0, 0xa1a1a1, 0xa2a2a2, 0xa3a3a3, 0xa4a4a4, 0xa5a5a5, 0xa6a6a6, 0xa7a7a7, 0xa8a8a8, 0xa9a9a9, 0xaaaaaa, 0xababab, 0xacacac, 0xadadad, 0xaeaeae, 0xafafaf, 0xb0b0b0, 0xb1b1b1, 0xb2b2b2, 0xb3b3b3, 0xb4b4b4, 0xb5b5b5, 0xb6b6b6, 0xb7b7b7, 0xb8b8b8, 0xb9b9b9, 0xbababa, 0xbbbbbb, 0xbcbcbc, 0xbdbdbd, 0xbebebe, 0xbfbfbf, 0xc0c0c0, 0xc1c1c1, 0xc2c2c2, 0xc3c3c3, 0xc4c4c4, 0xc5c5c5, 0xc6c6c6, 0xc7c7c7, 0xc8c8c8, 0xc9c9c9, 0xcacaca, 0xcbcbcb, 0xcccccc, 0xcdcdcd, 0xcecece, 0xcfcfcf, 0xd0d0d0, 0xd1d1d1, 0xd2d2d2, 0xd3d3d3, 0xd4d4d4, 0xd5d5d5, 0xd6d6d6, 0xd7d7d7, 0xd8d8d8, 0xd9d9d9, 0xdadada, 0xdbdbdb, 0xdcdcdc, 0xdddddd, 0xdedede, 0xdfdfdf, 0xe0e0e0, 0xe1e1e1, 0xe2e2e2, 0xe3e3e3, 0xe4e4e4, 0xe5e5e5, 0xe6e6e6, 0xe7e7e7, 0xe8e8e8, 0xe9e9e9, 0xeaeaea, 0xebebeb, 0xececec, 0xededed, 0xeeeeee, 0xefefef, 0xf0f0f0, 0xf1f1f1, 0xf2f2f2, 0xf3f3f3, 0xf4f4f4, 0xf5f5f5, 0xf6f6f6, 0xf7f7f7, 0xf8f8f8, 0xf9f9f9, 0xfafafa, 0xfbfbfb, 0xfcfcfc, 0xfdfdfd, 0xfefefe, 0xffffff },
		BMP_4BIT_COLOR_TABLE[] = { 0, 0x111111, 0x222222, 0x333333, 0x444444, 0x555555, 0x666666, 0x777777, 0x888888, 0x999999, 0xaaaaaa, 0xbbbbbb, 0xcccccc, 0xdddddd, 0xeeeeee, 0xffffff },
		BMP_1BIT_COLOR_TABLE[] = { 0, 0xffffff };

	// write out header & color space info
	if (header.bpp == 1) {
		header.offset += sizeof(BMP_1BIT_COLOR_SPACE_HEADER) + sizeof(BMP_COLOR_SPACE_INFO) + sizeof(BMP_1BIT_COLOR_TABLE);
		header.size += header.offset;
		header.headerSize += sizeof(BMP_COLOR_SPACE_INFO);
		out.write((const char*)&header, sizeof(header));
		out.write((const char*)BMP_1BIT_COLOR_SPACE_HEADER, sizeof(BMP_1BIT_COLOR_SPACE_HEADER));
		out.write((const char*)BMP_COLOR_SPACE_INFO, sizeof(BMP_COLOR_SPACE_INFO));
		out.write((const char*)BMP_1BIT_COLOR_TABLE, sizeof(BMP_1BIT_COLOR_TABLE));
	}
	else if (header.bpp == 4) {
		header.offset += sizeof(BMP_4BIT_COLOR_SPACE_HEADER) + sizeof(BMP_COLOR_SPACE_INFO) + sizeof(BMP_4BIT_COLOR_TABLE);
		header.size += header.offset;
		header.headerSize += sizeof(BMP_COLOR_SPACE_INFO);
		out.write((const char*)&header, sizeof(header));
		out.write((const char*)BMP_4BIT_COLOR_SPACE_HEADER, sizeof(BMP_4BIT_COLOR_SPACE_HEADER));
		out.write((const char*)BMP_COLOR_SPACE_INFO, sizeof(BMP_COLOR_SPACE_INFO));
		out.write((const char*)BMP_4BIT_COLOR_TABLE, sizeof(BMP_4BIT_COLOR_TABLE));
	}
	else if (header.bpp == 8) {
		header.offset += sizeof(BMP_8BIT_COLOR_SPACE_HEADER) + sizeof(BMP_COLOR_SPACE_INFO) + sizeof(BMP_8BIT_COLOR_TABLE);
		header.size += header.offset;
		header.headerSize += sizeof(BMP_COLOR_SPACE_INFO);
		out.write((const char*)&header, sizeof(header));
		out.write((const char*)BMP_8BIT_COLOR_SPACE_HEADER, sizeof(BMP_8BIT_COLOR_SPACE_HEADER));
		out.write((const char*)BMP_COLOR_SPACE_INFO, sizeof(BMP_COLOR_SPACE_INFO));
		out.write((const char*)BMP_8BIT_COLOR_TABLE, sizeof(BMP_8BIT_COLOR_TABLE));
	}
	else if (header.bpp == 24) {
		header.offset += sizeof(BMP_24BIT_COLOR_SPACE_HEADER) + sizeof(BMP_COLOR_SPACE_INFO);
		header.size += header.offset;
		header.headerSize += sizeof(BMP_COLOR_SPACE_INFO);
		out.write((const char*)&header, sizeof(header));
		out.write((const char*)BMP_24BIT_COLOR_SPACE_HEADER, sizeof(BMP_24BIT_COLOR_SPACE_HEADER));
		out.write((const char*)BMP_COLOR_SPACE_INFO, sizeof(BMP_COLOR_SPACE_INFO));
	}
	else if (header.bpp == 32) {
		header.compression = BMP_32BIT_COMPRESSION;
		header.offset += sizeof(BMP_32BIT_COLOR_SPACE_HEADER) + sizeof(BMP_COLOR_SPACE_INFO);
		header.size += header.offset;
		header.headerSize += sizeof(BMP_COLOR_SPACE_INFO);
		out.write((const char*)&header, sizeof(header));
		out.write((const char*)BMP_32BIT_COLOR_SPACE_HEADER, sizeof(BMP_32BIT_COLOR_SPACE_HEADER));
		out.write((const char*)BMP_COLOR_SPACE_INFO, sizeof(BMP_COLOR_SPACE_INFO));
	}

	// writing cycle
	char pad[3] = {0, 0, 0};
	for (int y = height - 1; y >= 0; --y) {
		pixbyte const* ptr = (pixbyte const*)pixels + y * rowSize;
		if (bpp == 32)
			for (int x = 0; x < width; ++x, ptr+=4)
				out << ptr[CHANNELS_4.A] << ptr[CHANNELS_4.B] << ptr[CHANNELS_4.G] << ptr[CHANNELS_4.R];
		else if (bpp == 24)
			for (int x = 0; x < width; ++x, ptr+=3)
				out << ptr[CHANNELS_3.B] << ptr[CHANNELS_3.G] << ptr[CHANNELS_3.R];
		else if (bpp == 4)
			for (int i = 0; i < rowSize; ++i, ptr++)
				out << LSN_MSN_REVERSE_LOOKUP[*ptr];
		else if (bpp == 1)
			for (int i = 0; i < rowSize; ++i, ptr++)
				out << LSB_MSB_REVERSE_LOOKUP[*ptr];
		else {
			out.write((const char*)ptr, rowSize);
		}
		out.write(pad, rowAlign);
	}
}
