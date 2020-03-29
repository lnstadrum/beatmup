#include "wav_utilities.h"

using namespace Beatmup;
using namespace WAV;


void WavHeader::set(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channelCount, uint32_t dataSize) {
    m_RIFF = __RIFF;
    m_WAVE = __WAVE;
    m_fmt_ = __fmt_;
    m_data = __data;
    __16 = 16;
    audioFormat = 1;
    this->sampleRate = sampleRate;
    this->bitsPerSample = bitsPerSample;
    this->numChannels = channelCount;
    blockAlign = channelCount * bitsPerSample / 8;
    byteRate = sampleRate * blockAlign;
    dataSizeBytes = 0;
    this->dataSizeBytes = dataSize;
    this->chunkSize = sizeof(WavHeader) + dataSize - 8;
}


void InvalidWavFile::check(WavHeader& header) {
    if (header.m_RIFF != WavHeader::__RIFF)
        throw InvalidWavFile("Incorrect WAV file: 'RIFF' marker missing");

    if (header.m_WAVE != WavHeader::__WAVE)
        throw InvalidWavFile("Incorrect WAV file: 'WAVE' marker missing");

    if (header.m_fmt_ != WavHeader::__fmt_)
        throw InvalidWavFile("Incorrect WAV file: 'fmt ' marker missing");

    if (header.m_data != WavHeader::__data)
        throw InvalidWavFile("Incorrect WAV file: 'data' marker missing");

    if (header.__16 != 16)
        throw InvalidWavFile("Incorrect WAV file: bad subchunk size");

    if (header.audioFormat != 1)
        throw InvalidWavFile("Incorrect WAV file: unsupported audio format");

    if (sizeof(header) + header.dataSizeBytes != header.chunkSize + 8)
        throw InvalidWavFile("Incorrect WAV file: size mismatch");
}
