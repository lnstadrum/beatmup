#include "aaudio_playback.h"

#include <aaudio/AAudio.h>

using namespace Beatmup;
using namespace Audio;

static const int64_t ONE_SECOND_IN_NANOS = 1000 * 1000 * 1000;


aaudio_data_callback_result_t aaudioDataCallback(
        AAudioStream *stream,
        void *userData,
        void *audioData,
        int32_t numFrames
) {
    ((Android::AAudioPlayback*)userData)->pullBuffer((sample8*)audioData, numFrames);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}


class Android::AAudioPlayback::Backend {
private:
    AAudioStream *stream;

    inline void check(aaudio_result_t code, const char* message) {
        if (code < AAUDIO_OK) {
            std::string report(message);
            report = report + '\n' + AAudio_convertResultToText(code);
            throw Audio::PlaybackException(message, (int)code);
        }
    }

public:
    Backend(): stream(nullptr) {}

    ~Backend() {
        if (stream)
            AAudioStream_close(stream);
    }

    void initialize(BasicRealtimePlayback* frontend, AbstractPlayback::Mode mode) {
        // if there is already an open stream, close it
        if (stream) {
            check(AAudioStream_close(stream), "Error when closing stream");
            stream = nullptr;
        }

        // create builder
        AAudioStreamBuilder *builder;
        check(AAudio_createStreamBuilder(&builder),
              "Error when creating stream builder.");

        // set mode
        AAudioStreamBuilder_setSampleRate(builder, mode.sampleRate);
        AAudioStreamBuilder_setChannelCount(builder, mode.numChannels);
        switch (mode.sampleFormat) {
            case Int16:
                AAudioStreamBuilder_setFormat(builder,  AAUDIO_FORMAT_PCM_I16);
                break;
            case Float32:
                AAudioStreamBuilder_setFormat(builder,  AAUDIO_FORMAT_PCM_FLOAT);
                break;
            default:
                throw PlaybackException("Sample format unsupported by AAudio.", 0, mode);

        }
        AAudioStreamBuilder_setBufferCapacityInFrames(builder, mode.bufferLength);

        // tweak performance
        AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
        AAudioStreamBuilder_setDataCallback(builder, aaudioDataCallback, frontend);
        AAudioStreamBuilder_setPerformanceMode(builder, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);

        // open stream
        check(AAudioStreamBuilder_openStream(builder, &stream),
                "Error when opening stream");

#ifdef BEATMUP_DEBUG
        BEATMUP_DEBUG_I("AAudio stream opened %d@%d Hz, %d samples:\n"
                        "  device id: %d\n"
                        "  sharing mode: %d\n"
                        "  performance mode: %d\n",
                        (int)mode.numChannels, (int)mode.sampleRate, (int)mode.bufferLength,
                        (int)AAudioStream_getDeviceId(stream),
                        (int)AAudioStream_getSharingMode(stream),
                        (int)AAudioStream_getPerformanceMode(stream));
#endif

        // clean up
        AAudioStreamBuilder_delete(builder);
    }


    void start() {
        check( AAudioStream_requestStart(stream),
                "Error when requesting start of a stream");

        aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_STARTED;
        check( AAudioStream_waitForStateChange(stream, AAUDIO_STREAM_STATE_STARTING, &nextState, ONE_SECOND_IN_NANOS),
                "Error when starting a stream");
    }


    void stop() {
        check( AAudioStream_requestStop(stream),
               "Error when requesting stop of a stream");

        aaudio_stream_state_t nextState = AAUDIO_STREAM_STATE_STOPPED;
        check( AAudioStream_waitForStateChange(stream, AAUDIO_STREAM_STATE_STOPPING, &nextState, ONE_SECOND_IN_NANOS),
               "Error when stopping a stream");
    }
};


Android::AAudioPlayback::AAudioPlayback(): BasicRealtimePlayback(OutputMode::PULL) {
    backend = new Backend();
}

Android::AAudioPlayback::~AAudioPlayback() {
    delete backend;
}

void Android::AAudioPlayback::initialize(AbstractPlayback::Mode mode) {
    BasicRealtimePlayback::initialize(mode);
    backend->initialize(this, mode);
}

void Android::AAudioPlayback::start() {
    BasicRealtimePlayback::start();
    backend->start();
}

void Android::AAudioPlayback::stop() {
    backend->stop();
}
