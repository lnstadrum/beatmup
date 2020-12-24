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

#include "../../parallelism.h"
#include "../../exception.h"
#include "../source.h"
#include "../sample_arithmetic.h"

namespace Beatmup {
    namespace Audio {

        /**
            Abstract audio playback base class.
            An AbstractTask incarnation that samples a given Source in a buffered fashion.
            The further processing of the sampled signal (e.g., sending to a device or writing to file) is done in subclasses.
        */
        class AbstractPlayback : public AbstractTask {
        public:
            /**
                Playback configuration
            */
            struct Mode {
                dtime sampleRate;                   //!< samples per second / sample rate in Hz
                AudioSampleFormat sampleFormat;     //!< format of each sample
                unsigned char numChannels;          //!< number of channels
                dtime bufferLength;                 //!< length of each atomic buffer in samples
                unsigned char numBuffers;           //!< number of atomic buffers

                /**
                    Sets playback configuration.
                    \param sampleRate        sampling frequency in Hz
                    \param format            sample format
                    \param numChannels       number of channels (e.g., 1 for mono, 2 for stereo)
                    \param bufferLength      length of a single buffer
                    \param numBuffers        number of buffers
                 */
                Mode(dtime sampleRate, AudioSampleFormat format, int numChannels, dtime bufferLength, int numBuffers);
                Mode(dtime sampleRate, AudioSampleFormat format, int numChannels, dtime bufferLength);
                Mode();

                /**
                    \return mode latency in samples.
                 */
                inline dtime getLatency() const { return bufferLength * numBuffers; }
            };

        protected:
            Mode mode;
            Source* source;
            AbstractPlayback();
            dtime clock;

            /**
                Moves time pointer one buffer forward
            */
            void advanceTime();

        public:
            ThreadIndex getMaxThreads() const;
            void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);

            /**
                Initializes the playback setting its main parameters
             */
            virtual void initialize(Mode mode);

            /**
                Specifies a Source to sample.
                \param[in] source   The source of audio signal to play from
            */
            inline void setSource(Source* source) { this->source = source; }

            /**
                Returns the signal source to sample.
            */
            inline Source* getSource() const { return source; }
        };

        /**
            Communicates an error occurred during the playback
        */
        class PlaybackException : public Exception {
        public:
            PlaybackException(const char *message, int resultCode);
            PlaybackException(const char *message, int resultCode, const AbstractPlayback::Mode& mode);
        };
    }
}
