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
#include "../fragments/sequence.h"
#include "../context.h"
#include "sample_arithmetic.h"
#include "source.h"
#include "../utils/input_stream.h"

namespace Beatmup {

/**
    Sound I/O and processing API.
*/
namespace Audio {

    /**
        An audio signal.
    */
    class Signal : public Fragments::SequenceToolkit<Signal> {
    private:
        /**
            Implements a Sequence::Pointer for audio signals
        */
        class Pointer : public Sequence::Pointer {
        public:
            Pointer(Signal& signal, dtime time, bool writing);
            virtual void releaseBuffer();
            unsigned char getChannelCount() const;
        };

    public:

        /**
            Provides reading access to the signal
        */
        class Reader : public Pointer {
        public:
            Reader(Signal& signal, dtime time);
            dtime acquireBuffer(const void* &data);
        };

        /**
            Provides writing access to the signal
        */
        class Writer : public Pointer {
        public:
            Writer(Signal& signal, dtime time);
            dtime acquireBuffer(void* &data);
        };

        /**
            Signal dynamics meter
        */
        class Meter : public Pointer {
        public:
            /**
                Specifies how to compute signal dynamics (minima and maxima in a given period of time)
            */
            enum class MeasuringMode {
                /**
                    Just run across all samples.
                    This does not require neither precomputing nor extra memory, but slow as hell.
                */
                preciseUsingSamples,

                /**
                    Find an approximated range using a lookup tree.
                    It is guaranteed that the resulting approximated dynamic range covers the precise range. Very fast for less
                    fragmented signals, does not hit the sample data in memory, but requires precomputing and increases the
                    memory footprint.
                */
                approximateUsingLookup,

                /**
                    Use lookup and then precise the measurement using sample data.
                    Generally fast, except highly fragmented signals. Requires precomputing and increases the memory footprint
                    (same as `approximateUsingLookup`);
                */
                preciseUsingLookupAndSamples
            };

        private:
            /**
                Measures signal dynamics in a given period of time.
                Deals with the fragmentation. The sample type must be coherent to the actual sample format of the signal.
                \param len          Period length in samples
                \param resolution   Number of output points
                \param min          Channelwise multiplexed magnitude minima, (resolution) points per channel
                \param max          Channelwise multiplexed magnitude maxima, (resolution) points per channel
            */
            template<typename sample> void measureInFragments(dtime len, int resolution, sample* min, sample* max);

            static void prepare(Signal& signal, int skipOnStart = 0, int numSteps = 1);

            MeasuringMode mode;

        public:
            /**
                Precomputes the dynamics lookup all over the signal, where needed
                \param signal       The signal to process
                \param runTask      If `true`, a task will be run in the context of the signal allowing to process it in parallel;
                                    otherwise the computation is done directly in the calling thread
            */
            static void prepareSignal(Signal& signal, bool runTask = true);

            /**
                Constructs a new meter
                \param signal       The signal to measure
                \param time         Initial time to start measurements from
                \param mode         Algorithm used to measure the signal dynamics
            */
            Meter(Signal& signal, dtime time, MeasuringMode mode = MeasuringMode::approximateUsingLookup);

            /**
                Measures signal dynamics in a given period of time.
                \param len          Period length in samples
                \param resolution   Number of output points
                \param min          Channelwise multiplexed magnitude minima, (resolution) points per channel
                \param max          Channelwise multiplexed magnitude maxima, (resolution) points per channel
            */
            template<typename sample> void measure(dtime len, int resolution, sample min[], sample max[]);

            inline void setMode(MeasuringMode newMode) { mode = newMode; }
        };

        /**
            Audio::Source reading samples from a given Signal.
         */
        class Source : public Audio::Source {
        private:
            Signal* signal;
            dtime time;
            AudioSampleFormat sampleFormat;
            unsigned char numChannels;

        public:
            Source(Signal&);

            ThreadIndex getMaxThreads() { return 1; }

            void prepare(
                const dtime sampleRate,
                const AudioSampleFormat sampleFormat,
                const unsigned char numChannels,
                const dtime maxBufferLen
            );

            void setClock(dtime time);

            void render(
                TaskThread& thread,
                sample8* buffer,
                const dtime bufferLength
            );
        };

        /**
            Communicates an error when inserting a incompatible fragment into a Signal
        */
        class IncompatibleFormat : public Exception {
        public:
            AudioSampleFormat sourceFormat, destFormat;
            int sourceChannelCount, destChannelCount;
            IncompatibleFormat(const Signal& source, const Signal& dest);
        };

    private:
        Context& ctx;

        AudioSampleFormat format;			//!< sample format

        const dtime defaultFragmentSize;
        int sampleRate;						//!< sampling frequency
        unsigned char channelCount;			//!< number of channels

    protected:
        virtual Signal* createEmpty() const;

    public:
        static const int DEFAULT_FRAGMENT_LENGTH_SEC = 5;

        /**
            Creates an empty signal
            \param context                   A Context instance the new signal is associated to
            \param format                    Sample format of the new signal
            \param sampleRate                Sample rate, Hz
            \param channels                  Number of channels
            \param defaultFragmentLenSec     Default fragment length in seconds
         */
        Signal(Context& context, AudioSampleFormat format, int sampleRate, unsigned char channels, float defaultFragmentLenSec = DEFAULT_FRAGMENT_LENGTH_SEC);

        /**
            Inserts a Signal into the current signal at a specific time moment
            \param signal       The signal to insert
            \param time         The time moment to insert the signal at
        */
        void insert(const Signal& signal, dtime time);

        /**
            Prolongates the sequence if necessary, to ensure given length
        */
        void reserve(dtime length);

        /**
            Stores the signal to a PCM-encoded WAV file
        */
        void saveWAV(const char* filename);

        static Signal* loadWAV(Context& ctx, const char* fileName);
        static Signal* loadWAV(Context& ctx, InputStream& inputStream);

        /**
            \return number of channels in the signal.
        */
        unsigned char getChannelCount() const { return channelCount; }

        /**
            \return sample format of the signal.
        */
        AudioSampleFormat getSampleFormat() const { return format; }

        /**
            \return a Context instance the signal is attached to.
        */
        Context& getContext() const { return ctx; }
    };

}
}
