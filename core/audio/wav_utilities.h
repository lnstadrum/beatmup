#pragma once
#include <stdint.h>
#include "../exception.h"

namespace Beatmup {
  namespace WAV {

    class WavHeader {
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
        dataSizeBytes;    // remainig size in bytes

      static const uint32_t
        __RIFF = 1179011410,
        __WAVE = 1163280727,
        __fmt_ = 544501094,
        __data = 1635017060;

      void set(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channelCount, uint32_t dataSize);
    };


    class InvalidWavFile: public Exception {
    public:
      InvalidWavFile(const char *message) : Exception(message) {};
      static void check(WavHeader& header);
    };
  }
}
