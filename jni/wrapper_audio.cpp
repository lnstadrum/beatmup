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

#include "wrapper.h"

#include "include/Beatmup_Audio_Signal.h"
#include "include/Beatmup_Audio_SignalPlot.h"
#include "include/Beatmup_Audio_Playback.h"
#include "include/Beatmup_Audio_HarmonicSource.h"

#include <core/audio/playback/android/sles_playback.h>
#include <core/audio/signal.h>
#include <core/audio/signal_plot.h>
#include <core/color/packing.h>

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          SIGNAL
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newAudioSignal, Java_Beatmup_Audio_Signal, newAudioSignal)
    (JNIEnv * jenv, jclass, jobject jCtx, jint format, jint samplerate, jint channels, jfloat fragment)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Context, ctx, jCtx);
    return (jlong) new Beatmup::Audio::Signal(*ctx, (Beatmup::AudioSampleFormat)format, samplerate, channels, fragment);
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
    BEATMUP_OBJ(Beatmup::Context, ctx, jCtx);
    try {
        return (jlong) Beatmup::Audio::Signal::loadWAV(*ctx, filename.c_str());
    }
    catch (Beatmup::Exception& ex) { $pool.throwToJava(jenv, "java/io/IOError", ex.what()); }
    return BeatmupJavaObjectPool::INVALID_HANDLE;
}


JNIMETHOD(jlong, newAudioSignalSource, Java_Beatmup_Audio_Signal, newAudioSignalSource)
    (JNIEnv * jenv, jclass, jobject jCtx, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::Signal, signal, handle);
    return (jlong) new Beatmup::Audio::Signal::Source(*signal);
}


JNIMETHOD(jint, getDuration, Java_Beatmup_Audio_Signal, getDuration)
    (JNIEnv * jenv, jclass, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::Signal, signal, handle);
    return (jint)signal->getDuration();
}


JNIMETHOD(jint, getSampleFormat, Java_Beatmup_Audio_Signal, getSampleFormat)
    (JNIEnv * jenv, jclass, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::Signal, signal, handle);
    return (jint)signal->getSampleFormat();
}


JNIMETHOD(jint, getChannelCount, Java_Beatmup_Audio_Signal, getChannelCount)
    (JNIEnv * jenv, jclass, jlong handle)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::Signal, signal, handle);
    return signal->getChannelCount();
}


/////////////////////////////////////////////////////////////////////////////////////////////
//                                        SIGNAL PLOT
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newSignalPlot, Java_Beatmup_Audio_SignalPlot, newSignalPlot)
    (JNIEnv * jenv, jclass, jobject jCtx)
{
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Audio::SignalPlot();
}


JNIMETHOD(void, prepareMetering, Java_Beatmup_Audio_SignalPlot, prepareMetering)
    (JNIEnv * jenv, jclass, jlong hSignal)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::Signal, signal, hSignal);
    Beatmup::Audio::Signal::Meter::prepareSignal(*signal, true);
}


JNIMETHOD(void, setSignal, Java_Beatmup_Audio_SignalPlot, setSignal)
    (JNIEnv * jenv, jobject, jlong hPlot, jlong hSignal)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::SignalPlot, plot, hPlot);
    BEATMUP_OBJ(Beatmup::Audio::Signal, signal, hSignal);
    plot->setSignal(signal);
}



JNIMETHOD(void, setBitmap, Java_Beatmup_Audio_SignalPlot, setBitmap)
    (JNIEnv * jenv, jobject, jlong hPlot, jobject jBitmap)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::SignalPlot, plot, hPlot);
    BEATMUP_OBJ(Beatmup::AbstractBitmap, bitmap, jBitmap);
    plot->setBitmap(bitmap);
}


JNIMETHOD(void, setWindow, Java_Beatmup_Audio_SignalPlot, setWindow)
    (JNIEnv * jenv, jobject, jlong hPlot, jint t1, jint t2, jint y1, jint y2, jfloat scale)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::SignalPlot, plot, hPlot);
    plot->setWindow(Beatmup::IntRectangle(t1, y1, t2, y2), scale);
}


JNIMETHOD(void, setPlotArea, Java_Beatmup_Audio_SignalPlot, setPlotArea)
    (JNIEnv * jenv, jobject, jlong hPlot, jint x1, jint y1, jint x2, jint y2)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::SignalPlot, plot, hPlot);
    plot->setPlotArea(Beatmup::IntRectangle(x1, y1, x2, y2));
}


JNIMETHOD(void, setPalette, Java_Beatmup_Audio_SignalPlot, setPalette)
    (JNIEnv * jenv, jobject, jlong hPlot, jint background, jint color1, jint color2)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::SignalPlot, plot, hPlot);
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
    BEATMUP_OBJ(Beatmup::Audio::SignalPlot, plot, hPlot);
    plot->setChannels(channels);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//                                          PLAYBACK
/////////////////////////////////////////////////////////////////////////////////////////////

JNIMETHOD(jlong, newPlayback, Java_Beatmup_Audio_Playback, newPlayback)
    (JNIEnv * jenv, jclass, jobject jCtx)
{
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Audio::Android::SLESPlayback();
}


JNIMETHOD(void, initialize, Java_Beatmup_Audio_Playback, initialize)
    (JNIEnv * jenv, jobject, jlong handle, jint sampleRate, jint sampleFormat, jint numChannels, jint bufferLength, jint numBuffers)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::AbstractPlayback, pb, handle);
    try {
        pb->initialize(Beatmup::Audio::AbstractPlayback::Mode(
           sampleRate, (Beatmup::AudioSampleFormat)sampleFormat, numChannels, bufferLength, numBuffers
        ));
    }
    catch (Beatmup::Audio::PlaybackException& pex) { $pool.throwToJava(jenv, "Beatmup/Exceptions/PlaybackException", pex.what()); }
}


JNIMETHOD(void, start, Java_Beatmup_Audio_Playback, start) (JNIEnv * jenv, jobject, jlong handle) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::BasicRealtimePlayback, pb, handle);
    try {
        pb->start();
    }
    catch (Beatmup::Audio::PlaybackException& pex) { $pool.throwToJava(jenv, "Beatmup/Exceptions/PlaybackException", pex.what()); }
}


JNIMETHOD(void, stop, Java_Beatmup_Audio_Playback, stop) (JNIEnv * jenv, jobject, jlong handle) {
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::BasicRealtimePlayback, pb, handle);
    try {
        pb->stop();
    }
    catch (Beatmup::Audio::PlaybackException& pex) { $pool.throwToJava(jenv, "Beatmup/Exceptions/PlaybackException", pex.what()); }
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

JNIMETHOD(jlong, newHarmonicSource, Java_Beatmup_Audio_HarmonicSource, newHarmonicSource)
    (JNIEnv *, jclass)
{
    BEATMUP_ENTER;
    return (jlong) new Beatmup::Audio::HarmonicSource();
}

JNIMETHOD(void, setFrequency, Java_Beatmup_Audio_HarmonicSource, setFrequency)
    (JNIEnv * jenv, jobject, jlong handle, jfloat hz)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::HarmonicSource, source, handle);
    source->setFrequency(hz);
}

JNIMETHOD(void, setPhase, Java_Beatmup_Audio_HarmonicSource, setPhase)
    (JNIEnv * jenv, jobject, jlong handle, jfloat rad)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::HarmonicSource, source, handle);
    source->setPhase(rad);
}

JNIMETHOD(void, setAmplitude, Java_Beatmup_Audio_HarmonicSource, setAmplitude)
    (JNIEnv * jenv, jobject, jlong handle, jfloat amp)
{
    BEATMUP_ENTER;
    BEATMUP_OBJ(Beatmup::Audio::HarmonicSource, source, handle);
    source->setAmplitude(amp);
}
