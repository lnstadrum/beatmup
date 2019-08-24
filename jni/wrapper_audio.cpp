#include "wrapper.h"

#include "jniheaders/Beatmup_Audio_Signal.h"
#include "jniheaders/Beatmup_Audio_Playback.h"
#include "jniheaders/Beatmup_Audio_Source_Harmonic.h"

#include "android/sles_playback.h"

#include <core/audio/audio_signal.h>

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          SIGNAL
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newAudioSignal, Java_Beatmup_Audio_Signal, newAudioSignal)
    (JNIEnv * jenv, jclass, jobject jCtx, jint format, jint samplerate, jint channels, jfloat fragment)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Environment, env, jCtx);
    return (jlong) new Beatmup::AudioSignal(*env, (Beatmup::AudioSampleFormat)format, samplerate, channels, fragment);
}


JNIMETHOD(jlong, newAudioSignalFromWAV, Java_Beatmup_Audio_Signal, newAudioSignalFromWAV)
    (JNIEnv * jenv, jclass, jobject jCtx, jstring jsFilename)
{
    // copy filename first
    const char* javaChar = jenv->GetStringUTFChars(jsFilename, 0);
    std::string filename(javaChar);
    jenv->ReleaseStringUTFChars(jsFilename, javaChar);
    // do the stuff then
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Environment, env, jCtx);
    BEATMUP_CATCH({
        return (jlong) Beatmup::AudioSignal::loadWAV(*env, filename.c_str());
    });
    return BeatmupJavaObjectPool::INVALID_HANDLE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          PLAYBACK
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newPlayback, Java_Beatmup_Audio_Playback, newPlayback)
    (JNIEnv * jenv, jclass, jobject jCtx)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Environment, env, jCtx);
    return (jlong) new Beatmup::Android::SLESPlayback();
}


JNIMETHOD(void, configure, Java_Beatmup_Audio_Playback, configure)
    (JNIEnv * jenv, jobject, jlong handle, jint sampleFormat, jint sampleRate, jint numChannels, jint numBuffers, jint bufferLength)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::AbstractPlayback, pb, handle);
    try {
        pb->initialize(Beatmup::Audio::AbstractPlayback::Mode(
           sampleRate, (Beatmup::AudioSampleFormat)sampleFormat, numChannels, bufferLength, numBuffers
        ));
    }
    catch (Beatmup::Audio::PlaybackException& pex) { $pool.throwToJava(jenv, "PlaybackException", pex.what()); }
    catch (std::exception& ex) { $pool.rethrowToJava(jenv, ex); }
}


JNIMETHOD(void, start, Java_Beatmup_Audio_Playback, start) (JNIEnv * jenv, jobject, jlong handle) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::BasicRealtimePlayback, pb, handle);
    try {
        pb->start();
    }
    catch (Beatmup::Audio::PlaybackException& pex) { $pool.throwToJava(jenv, "PlaybackException", pex.what()); }
    catch (std::exception& ex) { $pool.rethrowToJava(jenv, ex); }
}


JNIMETHOD(void, stop, Java_Beatmup_Audio_Playback, stop) (JNIEnv * jenv, jobject, jlong handle) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::BasicRealtimePlayback, pb, handle);
    try {
        pb->stop();
    }
    catch (Beatmup::Audio::PlaybackException& pex) { $pool.throwToJava(jenv, "PlaybackException", pex.what()); }
    catch (std::exception& ex) { $pool.rethrowToJava(jenv, ex); }
}


JNIMETHOD(void, setSource, Java_Beatmup_Audio_Playback, setSource)
    (JNIEnv * jenv, jobject, jlong handle, jobject jSource)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::AbstractPlayback, pb, handle);
    BEATMUP_OBJ(Beatmup::Audio::Source, source, jSource);
    pb->setSource(source);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                      HARMONIC SOURCE
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newHarmonicSource, Java_Beatmup_Audio_Source_Harmonic, newHarmonicSource)
    (JNIEnv *, jclass)
{
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Audio::HarmonicSource();
}

JNIMETHOD(void, setFrequency, Java_Beatmup_Audio_Source_Harmonic, setFrequency)
    (JNIEnv * jenv, jobject, jlong handle, jfloat hz)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::HarmonicSource, source, handle);
    source->setFrequency(hz);
}

JNIMETHOD(void, setPhase, Java_Beatmup_Audio_Source_Harmonic, setPhase)
    (JNIEnv * jenv, jobject, jlong handle, jfloat rad)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::HarmonicSource, source, handle);
    source->setPhase(rad);
}

JNIMETHOD(void, setAmplitude, Java_Beatmup_Audio_Source_Harmonic, setAmplitude)
    (JNIEnv * jenv, jobject, jlong handle, jfloat amp)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::HarmonicSource, source, handle);
    source->setAmplitude(amp);
}