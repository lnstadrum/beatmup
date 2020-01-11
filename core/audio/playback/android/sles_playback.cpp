#ifndef BEATMUP_PROFILE_NOAUDIO

#include "sles_playback.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

using namespace Beatmup;
using namespace Android;


void playerCallback(SLAndroidSimpleBufferQueueItf queue, void *data);


#undef assert

inline void assert(SLresult code, const char* message) {
    if (code != SL_RESULT_SUCCESS)
        throw Audio::PlaybackException(message, code);
}

inline void assert(SLresult code, const char* message, const Audio::AbstractPlayback::Mode& mode) {
    if (code != SL_RESULT_SUCCESS)
        throw Audio::PlaybackException(message, code, mode);
}


class SLESPlayback::SLESBackend {
private:
    SLEngineItf engine;
    SLObjectItf
            engineObj,
            outputMixObj,
            playerObj;

    SLPlayItf playbackObj;
    SLAndroidSimpleBufferQueueItf bufferQueueObj;

    msize bufferSize;
public:
    SLESBackend():
            engine(nullptr), engineObj(nullptr), outputMixObj(nullptr)
    {}

    ~SLESBackend() {
        //todo stop and destroy playback
    }


    inline void initialize(Audio::AbstractPlayback::Mode mode, Audio::BasicRealtimePlayback& playback) {
        SLresult result;

        // init engine if not yet
        if (!engineObj) {
            result = slCreateEngine(&engineObj, 0, NULL, 0, NULL, NULL);
            assert(result, "Engine creation failed");

            result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
            assert(result, "Engine realization failed");
        }

        if (!engine) {
            result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engine);
            assert(result, "Engine interface access failed");
        }

        // create output mix
        if (!outputMixObj) {
            result = (*engine)->CreateOutputMix(engine, &outputMixObj, 0, NULL, NULL);
            assert(result, "Output mix creation failed");
        }

        // realize the output mix
        result = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
        assert(result, "Output mix realization failed");

        // configure buffers queue
        SLDataLocator_AndroidSimpleBufferQueue bufferQueueConfig =
                {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, mode.numBuffers};

        // configure format
        SLDataFormat_PCM formatConfig = {
                SL_DATAFORMAT_PCM,
                mode.numChannels,
                (SLuint32) mode.sampleRate * 1000,      //in "milliHertz"
                (SLuint32) AUDIO_SAMPLE_SIZE[mode.sampleFormat] *8,
                (SLuint32) AUDIO_SAMPLE_SIZE[mode.sampleFormat] *8,
                0,
                SL_BYTEORDER_LITTLEENDIAN
        };
        SLDataSource audioSrc = {&bufferQueueConfig, &formatConfig};

        // configure audio sink
        SLDataLocator_OutputMix outputMixLocator = {SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
        SLDataSink audioSnk = {&outputMixLocator, NULL};

        // create audio player
        const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
        const SLboolean req[1] = {SL_BOOLEAN_TRUE};
        result = (*engine)->CreateAudioPlayer(engine, &playerObj, &audioSrc, &audioSnk, 1, ids, req);
        assert(result, "SLES playback initialization failed: buffered player creation", mode);

        // realize audio player
        result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);
        assert(result, "SLES playback initialization failed: buffered player realization", mode);

        // get playback control
        result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &playbackObj);
        assert(result, "SLES playback initialization failed: playback access", mode);

        // get buffer queue
        result = (*playerObj)->GetInterface(playerObj, SL_IID_BUFFERQUEUE, &bufferQueueObj);
        assert(result, "SLES playback initialization failed: buffered queue ccess", mode);

        // register callback
        result = (*bufferQueueObj)->RegisterCallback(bufferQueueObj, &playerCallback, &playback);
        assert(result, "SLES playback initialization failed: callback registration", mode);

        // store buffer size
        bufferSize = (msize) (mode.bufferLength * mode.numChannels * AUDIO_SAMPLE_SIZE[mode.sampleFormat]);
    }


    void start() {
        // start playing
        SLresult result = (*playbackObj)->SetPlayState(playbackObj, SL_PLAYSTATE_PLAYING);
        assert(result, "SLES playback error when starting");
    }


    void stop() {
        // start playing
        SLresult result = (*playbackObj)->SetPlayState(playbackObj, SL_PLAYSTATE_STOPPED);
        assert(result, "SLES playback error when stopping");
    }


    inline void enqueueBuffer(psample *buffa) {
        (*bufferQueueObj)->Enqueue(bufferQueueObj, buffa, bufferSize);
    }
};


void playerCallback(SLAndroidSimpleBufferQueueItf queue, void *data) {
    ((Audio::BasicRealtimePlayback*)data)->bufferQueueCallbackFunc();
}

SLESPlayback::SLESPlayback() {
    backend = new SLESBackend();
}

SLESPlayback::~SLESPlayback() {
    delete backend;
}

void SLESPlayback::initialize(Mode mode) {
    backend->initialize(mode, *this);
    Audio::BasicRealtimePlayback::initialize(mode);
}

void SLESPlayback::start() {
    backend->start();
}

void SLESPlayback::stop() {
    backend->stop();
}

void SLESPlayback::enqueueBuffer(psample *buffa, int bufferIndex) {
    backend->enqueueBuffer(buffa);
}

#endif