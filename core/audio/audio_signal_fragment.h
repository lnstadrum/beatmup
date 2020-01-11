/*
    Audio signal fragment definition.
    Represents a continuous piece of channelwise multiplexed signal. A lookup structure is implemented
    allowing to measure the signal dynamics efficiently.
    
    FOR INTERNAL USE, should not be included anywhere except audio_signal.cpp
*/

//#pragma once
#include "../environment.h"
#include "../exception.h"
#include "../fragments/fragment.h"
#include "sample_arithmetic.h"
#include "audio_signal.h"
#include <algorithm>


namespace Beatmup {

    /**
        A piece of sound!
    */
    class AudioSignalFragment : public Fragments::Fragment {
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
                \param channelCount			number of channels
                \param levelCount			number of detalization levels
                \param fineStepSize			the most detalized level step size in samples
                \param coarserStepSize		size of step in points for every upper (less detailed) level
            */
            void configureTree(unsigned char channelCount, int levelCount, int fineStepSize, int coarserStepSize);


            /**
                Updates tree from raw sample data
                \param data				pointer to the input data
                \param sampleCount		number of samples pointed by the data in each channel
            */
            template<typename sample> void updateTree(const sample* data, int sampleCount);


            /**
                Measures dynamics from time0 to time1 over ecah channels separately.
                \param time0		start time
                \param time1		stop time
                \param min			channelwise multiplexed minima
                \param max			channelwise multiplexed maxima
                \param data			channelwise multiplexed sample data for a more precise measurement; may be set to NULL
            */
            template<typename sample> void measure(dtime time0, dtime time1, sample* min, sample* max, const void* data) const;

                        
            inline bool isReady() const { return minmax != NULL; }
        };

    private:
        Environment& env;
        AudioSampleFormat format;
        unsigned char channelCount;				//!< number of channels
        unsigned char blockSize;				//!< size in bytes of a channelwise-multiplexed sample (block containing 1 sample per channel)
        memchunk data;

        struct Plot {
            DynamicsLookup lookup;
            int fineLevelStep, coarserLevelStep;
            Plot() : fineLevelStep(50), coarserLevelStep(10) {}
        } plot;

    public:
        AudioSignalFragment(Environment& env, AudioSampleFormat format, unsigned char channels, int samples);
        ~AudioSignalFragment();

        virtual AudioSignalFragment* clone() const;

        inline memchunk getData() const { return data; }
        inline const AudioSampleFormat getAudioFormat() const { return format; }
        inline Environment& getEnv() const { return env; }
        inline msize getSizeBytes() const { return getSampleCount() * blockSize; }
        inline unsigned char getBlockSize() const { return blockSize; }
        inline unsigned char getChannelCount() const { return channelCount; }

        void zero();

        inline bool isDynamicsLookupAvailable() const { return plot.lookup.isReady(); }
        void updateDynamicsLookup();
        
        /**
            Measures dynamics from time0 to time1 over ecah channels separately.
            \param time0		start time
            \param time1		stop time
            \param min			channelwise multiplexed minima
            \param max			channelwise multiplexed maxima
            \param data			channelwise multiplexed sample data for a more precise measurement; may be set to NULL
        */
        template<typename sample> void measureDynamics(int time0, int time1, sample* min, sample* max, AudioSignal::Meter::MeasuringMode mode);
    };

}