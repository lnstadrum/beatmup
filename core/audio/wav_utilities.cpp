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

#include "wav_utilities.h"

using namespace Beatmup::Audio::WAV;


void Header::set(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channelCount, uint32_t dataSize) {
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
    this->chunkSize = sizeof(Header) + dataSize - 8;
}


void InvalidWavFile::check(Header& header) {
    if (header.m_RIFF != Header::__RIFF)
        throw InvalidWavFile("Incorrect WAV file: 'RIFF' marker missing");

    if (header.m_WAVE != Header::__WAVE)
        throw InvalidWavFile("Incorrect WAV file: 'WAVE' marker missing");

    if (header.m_fmt_ != Header::__fmt_)
        throw InvalidWavFile("Incorrect WAV file: 'fmt ' marker missing");

    if (header.m_data != Header::__data)
        throw InvalidWavFile("Incorrect WAV file: 'data' marker missing");

    if (header.__16 != 16)
        throw InvalidWavFile("Incorrect WAV file: bad subchunk size");

    if (header.audioFormat != 1)
        throw InvalidWavFile("Incorrect WAV file: unsupported audio format");

    if (sizeof(header) + header.dataSizeBytes != header.chunkSize + 8)
        throw InvalidWavFile("Incorrect WAV file: size mismatch");
}
