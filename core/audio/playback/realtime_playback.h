/*
    Playback that performs in real time
*/
#pragma once

#include "abstract_playback.h"
#include <mutex>
#include <malloc.h>
#include <core/debug.h>

namespace Beatmup {
    namespace Audio {

        class BasicRealtimePlayback : public AbstractPlayback {
        private:
            psample** buffers;      //<! buffers containing the data to play
            int numBuffers;
            bool underrun;          //!< becomes true when underrun occurs
            unsigned int
                    fillIndex,          //!< points to the buffer currently being filled
                    sendIndex,          //!< points to the first buffer ready for playing
                    playIndex;          //!< points to the buffer currently being played; updated only in the callback
            std::mutex queueAccess;
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
            msize bufferSize;       //!< size of each buffer in bytes

            /**
             * Creates an atomic playing buffer
             * \param mode      the playback mode
             */
            virtual psample* createBuffer(const AbstractPlayback::Mode& mode) const;

            /**
             * Frees an atomic playing buffer
             */
            virtual void freeBuffer(psample* buffa) const;


            /**
             * Puts the buffer to send to the queue unconditionally
             */
            virtual void enqueueBuffer(psample *buffa, int bufferIndex) = 0;


        public:
            BasicRealtimePlayback();
            ~BasicRealtimePlayback();

            void initialize(Mode mode);
            bool process(TaskThread& thread);


            /**
             * Starts playback
             */
            virtual void start() = 0;


            /**
             * Stops playback
             */
            virtual void stop() = 0;


            /**
             * Returns the address of the buffer currently available for filling. If there is no such
             * buffer available, returns null.
             */
            psample* getCurrentBuffer() const;


            /**
             * Switches the current buffer being filled to the next one, if possible.
             * \returns `true` if possible.
             */
            bool goToNextBuffer();


            /**
             * Sends a filled buffer to the playing queue, if available.
             * \return `true` if sent.
             */
            bool sendNextBuffer();


            inline void bufferQueueCallbackFunc() {
                // a buffer is played, step forward
                BEATMUP_DEBUG_I("Buffer played: %d", playIndex);
                playIndex++;
                // send as many buffas as we can
                while (sendNextBuffer()) {}
                // check for underrun
                underrun = (playIndex >= sendIndex);
            }
        };

    }
}