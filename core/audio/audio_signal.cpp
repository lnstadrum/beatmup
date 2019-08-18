#include "audio_signal.h"
#include "audio_signal_fragment.h"
#include "wav_utilities.h"
#include "../exception.h"
#include <algorithm>
#include <fstream>

using namespace Beatmup;


AudioSignal* AudioSignal::createEmpty() const {
	return new AudioSignal(env, format, sampleRate, channelCount, (float)defaultFragmentSize / sampleRate);
}


AudioSignal::AudioSignal(Environment& env, AudioSampleFormat format, int sampleRate, unsigned char channels, float defaultFragmentLenSec) :
	env(env),
	format(format),
	sampleRate(sampleRate),
	channelCount(channels),
	defaultFragmentSize(floorf_fast(defaultFragmentLenSec * sampleRate))
{
	BEATMUP_ASSERT_DEBUG(defaultFragmentSize > 0);
}


void AudioSignal::insert(const AudioSignal& sequence, int time) {
	if (sequence.channelCount != channelCount || sequence.format != format)
		throw AudioSignal::IncompatibleFormat(sequence, *this);
	Sequence::insert(sequence, time);
}


void AudioSignal::reserve(int length) {
	int currentLength;
	while ((currentLength = getLength()) < length) {
		AudioSignalFragment* newbie = new AudioSignalFragment(env, format, channelCount, defaultFragmentSize);
		newbie->zero();
		concatenate(*newbie, 0, std::min(defaultFragmentSize, length - currentLength));
	}
}


AudioSignal* AudioSignal::loadWAV(Environment& env, const char* filename) {
	// open file
	std::ifstream file(filename, std::ifstream::in | std::ifstream::binary);
	if (!file.is_open())
		throw IOError(filename, "Unable to open for reading");

	// read header
	WAV::WAVHeader header;
	file.read((char*)&header, sizeof(header));	
	WAV::IncorrectWAV::check(header);

	// get sample format
	AudioSampleFormat format;
	switch (header.bitsPerSample) {
	case 8:
		format = Int8;
		break;
	case 16:
		format = Int16;
		break;
	case 32:
		format = Int32;
		break;
	default:
		throw WAV::IncorrectWAV("Incorrect WAV file: unsupported BPS");
	}

	// check number of channels (insanity check, basically)
	if (header.numChannels <= 0 || header.numChannels > 255)
		throw WAV::IncorrectWAV("Incorrect WAV file: unsupported channel count");

	// create signal
	AudioSignal* signal = new AudioSignal(env, format, header.sampleRate, header.numChannels);
	unsigned int totalTime = header.dataSizeBytes / (header.numChannels * header.bitsPerSample / 8);
	signal->reserve(totalTime);

	// read data using pointer
	Writer pointer(*signal, 0);
	while (pointer.hasData() && !file.eof()) {
		if (file.fail())
			throw IOError(filename, "Failed while reading");
		void* data;
		int length = pointer.acquireBuffer(data);		
		file.read((char*)data, length * header.numChannels * AUDIO_SAMPLE_SIZE[format]);
		pointer.releaseBuffer();
		pointer.jump(length);
	}

	return signal;
}


void AudioSignal::saveWAV(const char* filename) {
	// open file
	std::ofstream file(filename, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
	if (!file.is_open())
		throw IOError(filename, "Unable to open for writing");

	// init and write header
	unsigned char sampleSize = AUDIO_SAMPLE_SIZE[format];
	WAV::WAVHeader header;
	header.set(sampleRate, sampleSize * 8, channelCount, getLength() * channelCount * sampleSize);
	file.write((char*)&header, sizeof(header));
	
	// write out
	Reader pointer(*this, 0);
	while (pointer.hasData() && !file.eof()) {
		if (file.fail())
			throw IOError(filename, "Failed while writing");
		const void* data;
		int length = pointer.acquireBuffer(data);
		file.write((const char*)data, length * channelCount * sampleSize);
		pointer.releaseBuffer();
		pointer.jump(length);
	}
}


AudioSignal::Pointer::Pointer(AudioSignal& signal, int time, bool writing): Sequence::Pointer(signal, time, writing)
{}


void AudioSignal::Pointer::releaseBuffer() {
	AudioSignalFragment* fragment = (AudioSignalFragment*)pointer.fragment;
	fragment->getEnv().releaseMemory(fragment->getData(), false);
}


unsigned char AudioSignal::Pointer::getChannelCount() const {
	return ((AudioSignalFragment*)pointer.fragment)->getChannelCount();
}


AudioSignal::Reader::Reader(AudioSignal& signal, int time) : Pointer(signal, time, false) {};


int AudioSignal::Reader::acquireBuffer(const void* &data) {
	if (pointer.isNull()) {
		data = NULL;
		return 0;
	}
	AudioSignalFragment* fragment = (AudioSignalFragment*)pointer.fragment;
	data = (char*)fragment->getEnv().acquireMemory(fragment->getData()) + pointer.offset * fragment->getBlockSize();
	return pointer.length;
}


AudioSignal::Writer::Writer(AudioSignal& signal, int time) : Pointer(signal, time, true) {};

int AudioSignal::Writer::acquireBuffer(void* &data) {
	if (pointer.isNull()) {
		data = NULL;
		return 0;
	}
	AudioSignalFragment* fragment = (AudioSignalFragment*)pointer.fragment;
	data = (char*)fragment->getEnv().acquireMemory(fragment->getData()) + pointer.offset * fragment->getBlockSize();
	return pointer.length;
}


void AudioSignal::Writer::releaseBuffer() {
	AudioSignalFragment* fragment = (AudioSignalFragment*)pointer.fragment;
	if (fragment->isDynamicsLookupAvailable())
		fragment->updateDynamicsLookup();
	AudioSignal::Pointer::releaseBuffer();
}


void AudioSignal::Meter::prepare(AudioSignal& signal, int skipOnStart, int numSteps) {
	Meter me(signal, 0, MeasuringMode::approximateUsingLookup);
	for (int skip = 0; skip < skipOnStart; skip++)
		me.step();
	while (me.hasData()) {
		AudioSignalFragment* fragment = (AudioSignalFragment*)me.pointer.fragment;
		if (fragment && !fragment->isDynamicsLookupAvailable())
			fragment->updateDynamicsLookup();
		for (int skip = 0; skip < numSteps; skip++)
			me.step();
	}
}


AudioSignal::Meter::Meter(AudioSignal& signal, int time, MeasuringMode mode):
	Pointer(signal, time, false), mode(mode)
{}


void AudioSignal::Meter::prepareSignal(AudioSignal& signal, bool runTask) {
	if (runTask) {

		class DynamicsLookupPreparation : public AbstractTask {
		public:
			AudioSignal& signal;		
			bool process(TaskThread& thread) {
				// interleaving fragments among threads
				Meter::prepare(signal, thread.currentThread(), thread.totalThreads());
				return true;
			}

			ExecutionTarget getExecutionMode() const { return AbstractTask::ExecutionTarget::doNotUseGPU; }
			ThreadIndex maxAllowedThreads() const { return MAX_THREAD_INDEX; }
			DynamicsLookupPreparation(AudioSignal& signal) : signal(signal) {}
		};

		DynamicsLookupPreparation task(signal);
		signal.getEnvironment().performTask(task);
	}
	else
		Meter::prepare(signal);
}


template<typename sample> void AudioSignal::Meter::measureInFragments(int len, int resolution, sample* min, sample* max) {
	// The goal here is to deal with the fragmentation. Computation issues are processed on the fragment level.
	const int channelCount = ((AudioSignalFragment*)pointer.fragment)->getChannelCount();
	const dtime startTime = getTime();
	dtime t1 = startTime;
	sample *pMin = min, *pMax = max;

	// for all bins
	for (int bin = 1; bin <= resolution; bin++) {
		// determining bin bounds
		dtime t0 = t1;
		t1 = startTime + (long long)len * bin / resolution;

		// determining start time within fragment
		dtime fragStart = t0 - getTime() + pointer.offset;

		// initializing min and max values
		for (int ch = 0; ch < channelCount; ch++) {
			pMin[ch].x = sample::MAX_VALUE;
			pMax[ch].x = sample::MIN_VALUE;
		}

		// scanning fragments
		while (getTime() + pointer.length < t1) {
			((AudioSignalFragment*)pointer.fragment)->measureDynamics(fragStart, pointer.offset + pointer.length, pMin, pMax, mode);
			step();
			fragStart = pointer.offset;
		}
		((AudioSignalFragment*)pointer.fragment)->measureDynamics(fragStart, pointer.offset + t1 - getTime(), pMin, pMax, mode);

		// if no values defined
		for (int ch = 0; ch < channelCount; ch++)
			if (pMax[ch] < pMin[ch])
				pMin[ch].x = pMax[ch].x = 0;
		pMin += channelCount;
		pMax += channelCount;
	}
}

template void AudioSignal::Meter::measureInFragments(int len, int resolution, sample8* min, sample8* max);
template void AudioSignal::Meter::measureInFragments(int len, int resolution, sample16* min, sample16* max);
template void AudioSignal::Meter::measureInFragments(int len, int resolution, sample32* min, sample32* max);
template void AudioSignal::Meter::measureInFragments(int len, int resolution, sample32f* min, sample32f* max);


namespace Beatmup {		// why? Because of a bug in gcc

	template<> void AudioSignal::Meter::measure(int len, int resolution, sample16 min[], sample16 max[]) {
		const AudioSampleFormat fmt = ((AudioSignalFragment*)pointer.fragment)->getAudioFormat();
		if (fmt == Int16) {
			measureInFragments(len, resolution, (sample16*)min, (sample16*)max);
			return;
		}
		// The goal here is to deal with sample type transfer.
		const int size = resolution * ((AudioSignalFragment*)pointer.fragment)->getChannelCount();
		switch (fmt) {
		case Int8: {
			sample8 *tmin = new sample8[size], *tmax = new sample8[size];
			measureInFragments(len, resolution, tmin, tmax);
			convertSamples<sample8, sample16>(tmin, min, resolution);
			convertSamples<sample8, sample16>(tmax, max, resolution);
			delete[] tmin;
			delete[] tmax;
			break;
		}
		case Int32: {
			sample32 *tmin = new sample32[size], *tmax = new sample32[size];
			measureInFragments(len, resolution, tmin, tmax);
			convertSamples<sample32, sample16>(tmin, min, resolution);
			convertSamples<sample32, sample16>(tmax, max, resolution);
			delete[] tmin;
			delete[] tmax;
			break;
		}
		case Float32: {
			sample32f *tmin = new sample32f[size], *tmax = new sample32f[size];
			measureInFragments(len, resolution, tmin, tmax);
			convertSamples<sample32f, sample16>(tmin, min, resolution);
			convertSamples<sample32f, sample16>(tmax, max, resolution);
			delete[] tmin;
			delete[] tmax;
			break;
		}
		default:
			Insanity::insanity("Unknown audio format");
		}
	}
}


AudioSignal::IncompatibleFormat::IncompatibleFormat(const AudioSignal& source, const AudioSignal& dest):
	sourceFormat(source.getSampleFormat()), destFormat(dest.getSampleFormat()), sourceChannelCount(source.getChannelCount()), destChannelCount(dest.getChannelCount()),
	Exception(
		"Incompatible format: %s @ %d channels vs <-> %s @ %d channels",
		AUDIO_FORMAT_NAME[sourceFormat], sourceChannelCount,
		AUDIO_FORMAT_NAME[destFormat], destChannelCount
	)
{}