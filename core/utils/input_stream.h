/*
    Abstract thing to read data in a chunked fashion
*/
#pragma once
#include "../basic_types.h"

namespace Beatmup {
    class InputStream {
    public:
        virtual bool operator()(void * buffer, msize bytes) = 0;

        virtual bool eof() const = 0;
    };
}