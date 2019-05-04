#pragma once
#include "../exception.h"

namespace Beatmup {
	namespace WAV {
		
		class WAVHeader {
		public:
			unsigned int
				m_RIFF,				// "RIFF"
				chunkSize,			// file size minus 8,
				m_WAVE,				// "WAVE"
				m_fmt_,				// "fmt "
				__16;
			unsigned short int
				audioFormat,
				numChannels;		// number of channels
			unsigned int
				sampleRate,			// sampling frequency
				byteRate;
			unsigned short int
				blockAlign,
				bitsPerSample;		// bps
			unsigned int
				m_data,
				dataSizeBytes;		// remainig size in bytes

			static const int
				__RIFF = 1179011410,
				__WAVE = 1163280727,
				__fmt_ = 544501094,
				__data = 1635017060;

			void set(unsigned int sampleRate, unsigned char bitsPerSample, unsigned char channelCount, unsigned int dataSize);
		};


		class IncorrectWAV: public Exception {
		public:
			IncorrectWAV(const char *message) : Exception(message) {};
			static void check(WAVHeader& header);
		};
	}
}