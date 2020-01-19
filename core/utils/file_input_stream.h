/*

*/
#pragma once
#include "input_stream.h"
#include <fstream>

namespace Beatmup {
    class FileInputStream : public InputStream {
    private:
        std::ifstream stream;
    public:
        FileInputStream(const char* filename);
        bool isOpen() const;

        bool operator()(void * buffer, msize bytes);
        bool eof() const;
    };
}
