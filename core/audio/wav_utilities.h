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

#pragma once
#include <cstdint>
#include "../exception.h"

namespace Beatmup {
namespace Audio {

/**
    WAV files reading and writing
*/
namespace WAV {

    /**
        WAV file header
    */
    class Header {
    public:
        uint32_t
            m_RIFF,         // "RIFF"
            chunkSize,      // file size minus 8,
            m_WAVE,         // "WAVE"
            m_fmt_,         // "fmt "
            __16;
        uint16_t
            audioFormat,
            numChannels;    // number of channels
        uint32_t
            sampleRate,      // sampling frequency
            byteRate;
        uint16_t
            blockAlign,
            bitsPerSample;    // bps
        uint32_t
            m_data,
            dataSizeBytes;    // remaining size in bytes

        static const uint32_t
            __RIFF = 1179011410,
            __WAVE = 1163280727,
            __fmt_ = 544501094,
            __data = 1635017060;

        void set(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channelCount, uint32_t dataSize);
    };


    /**
        Communicates an error related to a WAV file content
    */
    class InvalidWavFile: public Exception {
    public:
        InvalidWavFile(const char *message) : Exception(message) {};
        static void check(Header& header);
    };
}
}
}
