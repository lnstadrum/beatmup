#include "wav_utilities.h"

using namespace Beatmup;
using namespace WAV;


void WAVHeader::set(unsigned int sampleRate, unsigned char bitsPerSample, unsigned char channelCount, unsigned int dataSize) {
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
	this->chunkSize = sizeof(WAVHeader) + dataSize - 8;
}


void IncorrectWAV::check(WAVHeader& header) {
	if (header.m_RIFF != WAVHeader::__RIFF)
		throw IncorrectWAV("Incorrect WAV file: 'RIFF' marker missing");
	
	if (header.m_WAVE != WAVHeader::__WAVE)
		throw IncorrectWAV("Incorrect WAV file: 'WAVE' marker missing");

	if (header.m_fmt_ != WAVHeader::__fmt_)
		throw IncorrectWAV("Incorrect WAV file: 'fmt ' marker missing");

	if (header.m_data != WAVHeader::__data)
		throw IncorrectWAV("Incorrect WAV file: 'data' marker missing");

	if (header.__16 != 16)
		throw IncorrectWAV("Incorrect WAV file: bad subchunk size");

	if (header.audioFormat != 1)
		throw IncorrectWAV("Incorrect WAV file: unsupported audio format");

	if (sizeof(header) + header.dataSizeBytes != header.chunkSize + 8)
		throw IncorrectWAV("Incorrect WAV file: size mismatch");
}