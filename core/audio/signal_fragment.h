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

//#pragma once
#include "../context.h"
#include "../memory.h"
#include "../exception.h"
#include "../fragments/fragment.h"
#include "sample_arithmetic.h"
#include "signal.h"
#include <algorithm>


namespace Beatmup {
namespace Audio {
    /**
        A piece of sound.
        Audio signals in Beatmup can be fragmented. SignalFragment is a continuous piece of audio signal in memory.
        A lookup structure is implemented allowing to measure the signal dynamics efficiently.

        FOR INTERNAL USE, should not be included anywhere except audio_signal.cpp
    */
    class SignalFragment : public Fragments::Fragment {
    private:

        /**
            Data structure allowing to plot efficiently audio signal graphs
        */
        class DynamicsLookup {
        private:
            DynamicsLookup* prev;				//!< previous (finer scale) level
            void *minmax;						//!< (2*size) points per channel, channel-wise multiplexed, interleaved minima and maxima
            unsigned char channelCount;
            int size;							//!< buffer size in points in one channel, i.e., size in bytes is 2*channelCount*sizeof(magnitude)*size
            int step;							//!< step (block) size in points w.r.t. previous layer (points = samples if there's no previous layer)
            int stepTime;						//!< step (block) size in absolute time units

            DynamicsLookup(const DynamicsLookup&) = delete;		//!< disabling copying constructor

        public:
            DynamicsLookup() : prev(nullptr), minmax(nullptr), channelCount(0), size(0), step(0), stepTime(0) {}
            ~DynamicsLookup();

            /**
                Frees the current level and all previous
            */
            void disposeTree();

            /**
                Sets up the tree structure
                \param channelCount			Number of channels
                \param levelCount			Number of detail levels
                \param fineStepSize			The most detailed level step size in samples
                \param coarserStepSize		Size of step in points for every upper (less detailed) level
            */
            void configureTree(unsigned char channelCount, int levelCount, int fineStepSize, int coarserStepSize);


            /**
                Updates tree from raw sample data
                \param data				Pointer to the input data
                \param sampleCount		Number of samples pointed by the data in each channel
            */
            template<typename sample> void updateTree(const sample* data, int sampleCount);


            /**
                Measures dynamics from time0 to time1 in each channels separately.
                \param time0		Start time
                \param time1		Stop time
                \param min			Channelwise multiplexed minima
                \param max			Channelwise multiplexed maxima
                \param data			Channelwise multiplexed sample data for a more precise measurement; may be null
            */
            template<typename sample> void measure(dtime time0, dtime time1, sample* min, sample* max, const void* data) const;


            inline bool isReady() const { return minmax != NULL; }
        };

    private:
        AudioSampleFormat format;
        unsigned char channelCount;				//!< number of channels
        unsigned char blockSize;				//!< size in bytes of a channelwise-multiplexed sample (block containing 1 sample per channel)
        AlignedMemory data;

        struct Plot {
            DynamicsLookup lookup;
            int fineLevelStep, coarserLevelStep;
            Plot() : fineLevelStep(50), coarserLevelStep(10) {}
        } plot;

    public:
        SignalFragment(AudioSampleFormat format, unsigned char channels, int samples);

        virtual SignalFragment* clone() const;

        inline sample8* getData() { return data.ptr<sample8>(); }
        inline const AudioSampleFormat getAudioFormat() const { return format; }
        inline msize getSizeBytes() const { return getSampleCount() * blockSize; }
        inline unsigned char getBlockSize() const { return blockSize; }
        inline unsigned char getChannelCount() const { return channelCount; }

        void zero();

        inline bool isDynamicsLookupAvailable() const { return plot.lookup.isReady(); }
        void updateDynamicsLookup();

        /**
            Measures dynamics from time0 to time1 in each channels separately.
            \param time0		Start time
            \param time1		Stop time
            \param min			Channelwise multiplexed minima
            \param max			Channelwise multiplexed maxima
            \param mode			Measurement mode
        */
        template<typename sample> void measureDynamics(int time0, int time1, sample* min, sample* max, Signal::Meter::MeasuringMode mode);
    };

}
}
