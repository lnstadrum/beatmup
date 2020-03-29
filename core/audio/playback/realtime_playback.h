/*
    Playback that performs in real time
*/
#pragma once

#include "abstract_playback.h"
#include "../../debug.h"
#include <mutex>
#include <malloc.h>

namespace Beatmup {
    namespace Audio {

        class BasicRealtimePlayback : public AbstractPlayback {
        protected:
            /**
             * The way the output audio signal is handled by the audio backend.
             */
            enum class OutputMode {
                PULL,       //!< The output is requested by pullOutput(..) called by the audio backend.
                PUSH,       //!< The output is sent to pushOutput(..) called by the rendering thread.
            };
        private:
            const OutputMode outputMode;

            sample8** buffers;          //!< buffers containing the data to play
            int numBuffers;
            int
                    fillIndex,          //!< points to the buffer currently being filled
                    sendIndex,          //!< points to the first buffer ready for playing
                    playIndex;          //!< points to the buffer currently being played; updated only in the callback

            dtime playingBufferOffset;     //!< when pulling output, offset in frames with respect to the currently playing buffer
            dtime skipFrames;              //!< when pulling output, number of frames to skip to give some time to the rendering process

            /*
                      playIndex ----> sendIndex ----> fillIndex
                                  |               |
                                queue           filled
                sendIndex = fillIndex: can you go faster, eh?
                fillIndex ~ playIndex: ok, take some rest
                playIndex = sendIndex: you're too slow for this job! (underrun)
             */


            /**
             * Allocates all the atomic buffers
             */
            void prepareBuffers(const AbstractPlayback::Mode& mode);

            /*
             * Deallocates all the atomic buffers
             */
            void freeBuffers();


        protected:
            BasicRealtimePlayback(OutputMode);

            msize bufferSize;       //!< size of each buffer in bytes

            /**
             * Creates an atomic playing buffer
             * \param mode      the playback mode
             */
            virtual sample8* createBuffer(const AbstractPlayback::Mode& mode) const;

            /**
             * Frees an atomic playing buffer
             */
            virtual void freeBuffer(sample8* buffer) const;


            /**
             * Pushes some data to the output
             */
            virtual void pushBuffer(sample8 *buffer, int bufferIndex) {};


        public:
            virtual ~BasicRealtimePlayback();

            void initialize(Mode mode);

            bool process(TaskThread& thread);


            /**
             * Starts playback
             */
            virtual void start();


            /**
             * Stops playback
             */
            virtual void stop() = 0;


            inline void bufferQueueCallbackFunc() {
                // a buffer is played, step forward
                //BEATMUP_DEBUG_I("Buffer played: %d", playIndex);
                playIndex++;
                // check for underrun
                //underrun = (playIndex >= sendIndex);
            }


            /**
             * Called in pulling output mode to send data to output.
             * \param buffer        Pointer to a buffer to put data to
             * \param numFrames     Number of "frames" (samples per single channel) to put to the buffer
             */
            void pullBuffer(sample8 *buffer, dtime numFrames);
        };

    }
}