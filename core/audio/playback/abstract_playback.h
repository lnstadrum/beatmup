/*
    Abstract audio playback
*/
#pragma once

#include "../../parallelism.h"
#include "../../exception.h"
#include "../source.h"
#include "../sample_arithmetic.h"

namespace Beatmup {
    namespace Audio {

        class AbstractPlayback : public AbstractTask {
        public:
            /**
             * Parameters describing playback mode
             */
            struct Mode {
                dtime sampleRate;                   //!< samples per second / sample rate in Hz
                AudioSampleFormat sampleFormat;     //!< format of each sample
                unsigned char numChannels;          //!< number of channels
                dtime bufferLength;                 //!< length of each atomic buffer in samples
                unsigned char numBuffers;           //!< number of atomic buffers

                Mode(dtime sampleRate, AudioSampleFormat format, int numChannels, dtime bufferLength, int numBuffers);
                Mode(dtime sampleRate, AudioSampleFormat format, int numChannels, dtime bufferLength);
                Mode();
            };

        protected:
            Mode mode;
            Source* source;
            AbstractPlayback();
            dtime clock;

            void advanceTime();

        public:
            /**
             * Initializes the playback by setting its main parameters
             * \param sampleRate        sampling frequency of the playback in Hz
             * \param sampleFormat      sample format of the playback
             * \param channelCount      number of channels
             */
            virtual void initialize(Mode mode);

            ThreadIndex maxAllowedThreads() const;
            void beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu);

            inline void setSource(Source* source) { this->source = source; }
            inline Source* getSource() const { return source; }
        };


        class PlaybackException : public Exception {
        public:
            PlaybackException(const char *message, unsigned int resultCode);
            PlaybackException(const char *message, unsigned int resultCode, const AbstractPlayback::Mode& mode);
        };
    }
}