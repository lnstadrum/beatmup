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

#include "../parallelism.h"
#include "sample_arithmetic.h"

namespace Beatmup {
    namespace Audio {

        /**
            Abstract source of audio signal. Produces samples in time, possibly in multiple threads running in parallel.
        */
        class Source {
        public:
            virtual ~Source() {};


            /**
                Prepares a source to render audio data. Called by the user before any rendering to communicate the
                configuration of the upcoming rendering process.
                \param sampleRate        The output sample rate in Hz
                \param sampleFormat      The output sample format
                \param numChannels       Number of channels in the output
                \param maxBufferLen      Maximum expected length of target audio buffers on rendering
                                         phase, in samples per one channel
             */
            virtual void prepare(
                    const dtime sampleRate,
                    const AudioSampleFormat sampleFormat,
                    const unsigned char numChannels,
                    const dtime maxBufferLen
            ) = 0;


            /**
             * Called by the source user when an abrupt time change occurs (e.g., due to seeking)
             */
            virtual void setClock(dtime time);


            /**
             * Returns the maximum number of working threads for rendering from this source. The actual number of
             * threads may be less than the one returned by this function. It is the source duty to deal with the actual
             * number of workers.
             */
            virtual ThreadIndex getMaxThreads() { return 1; }


            /**
             * Renders audio data to the target output buffer given by the user.
             * Called after at least one call to prepare(). The sampling parameters must match the ones communicated
             * on the preparation phase. The requested buffer length does not exceed the one set before. The time is
             * given by the clock set before, and with every call it advances by {bufferLength} samples.
             * \param thread            the task thread issuing this rendering call
             * \param buffer            a pointer to the beginning of a channelwise-multiplexed output buffer
             * \param bufferLength      the requested buffer length, in samples per single channel
             */
            virtual void render(
                    TaskThread& thread,
                    sample8* buffer,
                    const dtime bufferLength
            ) = 0;
        };


        /**
            A Source producing a sinusoidal signal, mainly for test purposes
         */
        class HarmonicSource : public Source {
        private:
            AudioSampleFormat sampleFormat;
            dtime sampleRate, time;
            float amplitude, frequency, phase;
            unsigned char numChannels;
        public:
            HarmonicSource() : amplitude(0.05), frequency(800), phase(0) {}

            void setFrequency(float hz) { frequency = hz; }
            float getFrequency() const { return frequency; }

            void setPhase(float radians) { phase = radians; }
            float getPhase() const { return phase; }

            void setAmplitude(float amp) { this->amplitude = amp; }
            float getAmplitude() const { return amplitude; }

            void prepare(
                    const dtime sampleRate,
                    const AudioSampleFormat sampleFormat,
                    const unsigned char numChannels,
                    const dtime maxBufferLen
            );

            void render(
                    TaskThread& thread,
                    sample8* buffer,
                    const dtime bufferLength
            );

            void setClock(dtime time);
        };
    }
}
