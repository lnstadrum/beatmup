#include "file_input_stream.h"

using namespace Beatmup;

FileInputStream::FileInputStream(const char *filename):
    stream(filename, std::ios::in | std::ios::binary)
{}


bool FileInputStream::isOpen() const {
    return stream.is_open();
}


bool FileInputStream::operator()(void *buffer, msize bytes) {
    stream.read((char*)buffer, bytes);
    return stream.good();
}

bool FileInputStream::eof() const {
    return stream.eof();
}
