#include "internal_bitmap.h"
#include "../exception.h"
#include <fstream>

using namespace Beatmup;

typedef struct __attribute__((packed)) {
	uint8_t magic[2];
	uint32_t size;
	uint16_t reserved[2];
	uint32_t offset;
	uint32_t headerSize;
	int32_t  width, height;
	uint16_t numColorPlanes;
	uint16_t bpp;
	uint32_t compression;
	uint32_t imageSize;
	int32_t  hdpi, vdpi;
	uint32_t numImportantColors;
} BmpHeader;


static const BmpHeader BMP_HEADER_REFERENCE = {
	{ 'B', 'M' },		// magic
	0, { 0, 0 }, 0,
	40,					// headerSize
	0, 0,
	1,					// numColorPlanes
	0,
	0,					// compression
	0, 0, 0, 0
};


InternalBitmap::InternalBitmap(Environment& env, PixelFormat pixelFormat, int width, int height) :
	AbstractBitmap(env),
	pixelFormat(pixelFormat), width(width), height(height),
	data(nullptr)
{
	if (getBitsPerPixel() < 8) {
		int n = 8 / getBitsPerPixel();
		this->width = ceili(width, n) * n;
	}
	memory = env.allocateMemory(getMemorySize());
}


InternalBitmap::InternalBitmap(Environment& env, const std::string& filename) :
	AbstractBitmap(env),
	data(nullptr)
{
	// open file
	std::ifstream in(filename, std::fstream::in | std::fstream::binary);
	if (!in.good())
		throw IOError(filename, "Cannot read.");

	// grab header & check some fields
	BmpHeader header;
	in.read((char*)&header, sizeof(header));
	if (header.magic[0] != BMP_HEADER_REFERENCE.magic[0]
		|| header.magic[1] != BMP_HEADER_REFERENCE.magic[1])
		throw IOError(filename, "Likely not a bmp file. Bad magic.");
	if (header.headerSize < BMP_HEADER_REFERENCE.headerSize
		|| header.numColorPlanes != BMP_HEADER_REFERENCE.numColorPlanes)
		throw IOError(filename, "Likely not a bmp file. Invalid header.");
	if (header.compression != BMP_HEADER_REFERENCE.compression)
		throw IOError(filename, "Compressed bitmaps are not supported.");

	// grab bitmap dimensions and format
	switch (header.bpp) {
		case 8:
			this->pixelFormat = PixelFormat::SingleByte;
		break;
		case 24:
			this->pixelFormat = PixelFormat::TripleByte;
		break;
		case 32:
			this->pixelFormat = PixelFormat::QuadByte;
		break;
		default:
			throw IOError(filename, "Unsupported pixel format");
	}
	this->width = header.width;
	this->height = header.height;

	// allocate memory
	memory = env.allocateMemory(getMemorySize());
	pixptr pixels = env.acquireMemory(memory);

	// seek pixels
	in.seekg(header.offset, std::ios_base::beg);

	// reading cycle
	const msize rowSize = ceili(width * AbstractBitmap::BITS_PER_PIXEL[pixelFormat], 8);
	char pad[3];
	for (int i = 0; i < height && !in.eof(); ++i) {
		in.read((char*)pixels, rowSize);
		pixels += rowSize;
		in.read(pad, rowSize - rowSize / 4 * 4);
	}

	// release memory
	env.releaseMemory(memory, false);
}


InternalBitmap::~InternalBitmap() {
	env.freeMemory(memory);
}


const PixelFormat InternalBitmap::getPixelFormat() const {
	return pixelFormat;
}


const int InternalBitmap::getWidth() const {
	return width;
}


const int InternalBitmap::getHeight() const {
	return height;
}


const msize InternalBitmap::getMemorySize() const {
	// a proper way to compute required memory size (works for bpp < 8)
	return ceili(height * width * AbstractBitmap::BITS_PER_PIXEL[pixelFormat], 8);
}


const pixptr InternalBitmap::getData(int x, int y) const {
	return data + (y * width + x) * AbstractBitmap::BITS_PER_PIXEL[pixelFormat] / 8;
}


void InternalBitmap::lockPixelData() {
	if (!data)
		data = env.acquireMemory(memory);
}


void InternalBitmap::unlockPixelData() {
	if (data) {
		data = nullptr;
		env.releaseMemory(memory, !isUpToDate(ProcessingTarget::CPU));
	}
}


void InternalBitmap::unlockPixels() {
	AbstractBitmap::unlockPixels();
	/*if (isUpToDate(ProcessingTarget::GPU))
		// if it is not a mask, lets free CPU
		if (!isMask()) {
			invalidate(ProcessingTarget::CPU);
			// this will cause freeing the memory when releasing the chunk
		}*/
}


void InternalBitmap::saveBmp(const std::string& filename) {
	// setup header
	BmpHeader header = BMP_HEADER_REFERENCE;
	switch (pixelFormat) {
		case PixelFormat::SingleByte:
			header.bpp = 8;
		break;
		case PixelFormat::TripleByte:
			header.bpp = 24;
		break;
		case PixelFormat::QuadByte:
			header.bpp = 32;
		break;
		default:
			throw IOError(filename, "Unsupported pixel format");
	}

	header.width = width;
	header.height = height;
	header.offset = sizeof(header);

	// setup output stream
	std::fstream out(filename, std::ios::out | std::ios::binary);
	if (!out.good())
		throw IOError(filename, "Cannot write to file.");
	out.write((const char*)&header, sizeof(header));

	// writing cycle
	pixptr pixels = env.acquireMemory(memory);
	const msize rowSize = ceili(width * AbstractBitmap::BITS_PER_PIXEL[pixelFormat], 8);
	char pad[3] = {0, 0, 0};
	for (int i = 0; i < height; ++i) {
		out.write((const char*)pixels, rowSize);
		pixels += rowSize;
		out.write(pad, ceili(rowSize, 4) * 4 - rowSize);
	}
	env.releaseMemory(memory, false);
}
