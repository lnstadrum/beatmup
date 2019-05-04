/*
    An abstract source of audio signal
*/
#pragma once

#include "../parallelism.h"
#include "sample_arithmetic.h"

namespace Beatmup {
    namespace Audio {
        class Source {
        public:
            virtual ~Source() {};


            /**
             * Prepares a source to render audio data. Called by the source userbefore any rendering
             * to communicate to the source the requested rendering mode.
             * \param sampleRate        the output sample rate in Hz
             * \param sampleFormat      the output sample format
             * \param numChannels       number of channels in the output
             * \param maxBufferLen      maximum expected length of target audio buffers on rendering
             *                          phase, in samples per one channel
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
             * Returns the maximum number of working threads for rendering from this source.
             * The actual number of threads may be less than the one returned by this function. It
             * is the responsibility of the source to deal with the actual number of workers.
             */
            virtual ThreadIndex maxAllowedThreads() { return 1; }


            /**
             * Renders audio data to the target output buffer given by the source user.
             * Called after at least one call to prepare(...). The sampling parameters must match
             * the ones communicated on the preparation phase. The requested buffer length does not
             * exceed the one set before. The time is given by the clock set before, and with each
             * call it advances on the {bufferLength} samples.
             * \param thread            the task thread issuing this rendering call
             * \param buffer            a pointer to the beginning of a channel-multiplexed output
             *                          buffer
             * \param bufferLength      the requested buffer length, in samples per one channel
             */
            virtual void render(
                    TaskThread& thread,
                    psample* buffer,
                    const dtime bufferLength
            ) = 0;
        };


        /**
         * A sinusoidal signal source, mainly for tes purposes
         */
        class HarmonicSource : public Source {
        private:
            AudioSampleFormat sampleFormat;
            dtime sampleRate, time;
            float amplitude, frequency, phase;
            unsigned char numChannels;
        public:
            HarmonicSource() : frequency(800), phase(0), amplitude(0.05) {}

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
                    psample* buffer,
                    const dtime bufferLength
            );

            void setClock(dtime time);
        };
    }
}