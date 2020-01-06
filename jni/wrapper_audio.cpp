#include "wrapper.h"

#include "jniheaders/Beatmup_Audio_Signal.h"
#include "jniheaders/Beatmup_Audio_SignalPlot.h"
#include "jniheaders/Beatmup_Audio_Playback.h"
#include "jniheaders/Beatmup_Audio_Source_Harmonic.h"

#include "android/sles_playback.h"

#include <core/audio/audio_signal.h>
#include <core/audio/audio_signal_plot.h>
#include <core/color/packing.h>

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


JNIMETHOD(jlong, newAudioSignalSource, Java_Beatmup_Audio_Signal, newAudioSignalSource)
    (JNIEnv * jenv, jclass, jobject jCtx, jlong hSignal)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Environment, env, jCtx);
    BEATMUP_OBJ(Beatmup::AudioSignal, signal, hSignal);
    return (jlong) new Beatmup::AudioSignal::Source(*signal);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                        SIGNAL PLOT
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newSignalPlot, Java_Beatmup_Audio_SignalPlot, newSignalPlot)
    (JNIEnv * jenv, jclass, jobject jCtx)
{
    BEATMUP_ENTER;
    return (jlong) new Beatmup::AudioSignalPlot();
}


JNIMETHOD(void, prepareMetering, Java_Beatmup_Audio_SignalPlot, prepareMetering)
    (JNIEnv * jenv, jclass, jlong hSignal)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignal, signal, hSignal);
    Beatmup::AudioSignal::Meter::prepareSignal(*signal, true);
}


JNIMETHOD(void, setSignal, Java_Beatmup_Audio_SignalPlot, setSignal)
    (JNIEnv * jenv, jobject, jlong hPlot, jlong hSignal)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignalPlot, plot, hPlot);
    BEATMUP_OBJ(Beatmup::AudioSignal, signal, hSignal);
    plot->setSignal(signal);
}



JNIMETHOD(void, setBitmap, Java_Beatmup_Audio_SignalPlot, setBitmap)
    (JNIEnv * jenv, jobject, jlong hPlot, jobject jBitmap)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignalPlot, plot, hPlot);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    plot->setBitmap(bitmap);
}


JNIMETHOD(void, setWindow, Java_Beatmup_Audio_SignalPlot, setWindow)
    (JNIEnv * jenv, jobject, jlong hPlot, jint t1, jint t2, jint y1, jint y2, jfloat scale)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignalPlot, plot, hPlot);
    plot->setWindow(Beatmup::IntRectangle(t1, y1, t2, y2), scale);
}


JNIMETHOD(void, setPlotArea, Java_Beatmup_Audio_SignalPlot, setPlotArea)
    (JNIEnv * jenv, jobject, jlong hPlot, jint x1, jint y1, jint x2, jint y2)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignalPlot, plot, hPlot);
    plot->setPlotArea(Beatmup::IntRectangle(x1, y1, x2, y2));
}


JNIMETHOD(void, setPalette, Java_Beatmup_Audio_SignalPlot, setPalette)
    (JNIEnv * jenv, jobject, jlong hPlot, jint background, jint color1, jint color2)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignalPlot, plot, hPlot);
    plot->setPalette(
      Beatmup::fromPackedInt((int32_t)background),
      Beatmup::fromPackedInt((int32_t)color1),
      Beatmup::fromPackedInt((int32_t)color2)
    );
}


JNIMETHOD(void, setChannels, Java_Beatmup_Audio_SignalPlot, setChannels)
    (JNIEnv * jenv, jobject, jlong hPlot, jint channels)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::AudioSignalPlot, plot, hPlot);
    plot->setChannels(channels);
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
