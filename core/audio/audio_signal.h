#pragma once
#include "../fragments/sequence.h"
#include "../environment.h"
#include "sample_arithmetic.h"
#include "source.h"

namespace Beatmup {

	class AudioSignal : public Fragments::SequenceToolkit<AudioSignal> {
	private:
		class Pointer : public Sequence::Pointer {
		public:
			Pointer(AudioSignal& signal, dtime time, bool writing);
			virtual void releaseBuffer();
			unsigned char getChannelCount() const;
		};

	public:

		/**
			Provides reading access to the signal
		*/
		class Reader : public Pointer {
		public:
			Reader(AudioSignal& signal, dtime time);
			dtime acquireBuffer(const void* &data);
		};

		/**
			Provides writing access to the signal
		*/
		class Writer : public Pointer {
		public:
			Writer(AudioSignal& signal, dtime time);
			dtime acquireBuffer(void* &data);
		};

		/**
			Signal dynamics meter
		*/
		class Meter : public Pointer {
		public:
			/**
				Specifies how to compute signal dynamics (minima and maxima in a given period of time)
			*/
			enum class MeasuringMode {
				/**
					Just run across all samples.
					This does not require neither precomputing nor extra memory, but slow as hell.
				*/
				preciseUsingSamples,

				/**
					Find an approximative range using a lookup tree.
					It is guaranteed that the resulting approximative dynamic range covers the precise range. Very fast for less
					fragmented signals, does not hit the sample data in memory, but requires precomputing and increases the
					memory footprint.
				*/
				approximateUsingLookup,

				/**
					Use lookup and then precise the measurement using sample data.
					Generally fast, except highly fragmented signals. Requires precomputing and increases the memory footprint
					(same as `approximativelyUsingLookup`);
				*/
				preciseUsingLookupAndSamples
			};

		private:
			/**
				Measures signal dynamics in a given period of time.
				Deals with the fragmentation. The sample type must be coherent to the actual sample format of the signal.
				\param len			period length in samples
				\param resolution	number of output points
				\param min			channelwise multiplexed magnitude minima, (resolution) points per channel
				\param max			channelwise multiplexed magnitude maxima, (resolution) points per channel
			*/
			template<typename sample> void measureInFragments(dtime len, int resolution, sample* min, sample* max);

			static void prepare(AudioSignal& signal, int skipOnStart = 0, int numSteps = 1);

			MeasuringMode mode;

		public:
			/**
				Precomputes the dynamics lookup all over the signal, where needed
				\param signal		the signal to process
				\param runTask		if `true`, a task will be run in the environment of the signal allowing to process it in parallel;
									otherwise the computation is done directly in the calling thread
			*/
			static void prepareSignal(AudioSignal& signal, bool runTask = true);

			/**
				Constructs a new meter
				\param signal		the signal to measure
				\param time			initial time to start measurements from
				\param mode			algorithm used to measure the signal dynamics
			*/
			Meter(AudioSignal& signal, dtime time, MeasuringMode mode = MeasuringMode::approximateUsingLookup);

			/**
				Measures signal dynamis in a given period of time.
				\param len			period length in samples
				\param resolution	number of output points
				\param min			channelwise multiplexed magnitude minima, (resolution) points per channel
				\param max			channelwise multiplexed magnitude maxima, (resolution) points per channel
			*/
			template<typename sample> void measure(dtime len, int resolution, sample min[], sample max[]);

			inline void setMode(MeasuringMode newMode) { mode = newMode; }
		};

		/**
		 * A Source using audio signal
		 */
		class Source : public Audio::Source {
		private:
			AudioSignal* signal;
			dtime time;
			AudioSampleFormat sampleFormat;
			unsigned char numChannels;

		public:
			Source(AudioSignal&);

			ThreadIndex maxAllowedThreads() { return 1; }

			void prepare(
				const dtime sampleRate,
				const AudioSampleFormat sampleFormat,
				const unsigned char numChannels,
				const dtime maxBufferLen
			);

			void setClock(dtime time);

			void render(
				TaskThread& thread,
				psample* buffer,
				const dtime bufferLength
			);
		};

		class IncompatibleFormat : public Exception {
		public:
			AudioSampleFormat sourceFormat, destFormat;
			int sourceChannelCount, destChannelCount;
			IncompatibleFormat(const AudioSignal& source, const AudioSignal& dest);
		};

	private:
		Environment& env;

		AudioSampleFormat format;			//!< sample format

		const dtime defaultFragmentSize;
		int sampleRate;						//!< sampling frequency
		unsigned char channelCount;			//!< number of channels

	protected:
		virtual AudioSignal* createEmpty() const;

	public:
		static const int DEFAULT_FRAGMENT_LENGTH_SEC = 5.0f;

		AudioSignal(Environment& env, AudioSampleFormat format, int sampleRate, unsigned char channels, float defaultFragmentLenSec = DEFAULT_FRAGMENT_LENGTH_SEC);

		void insert(const AudioSignal& sequence, dtime time);

		/**
			Prolongates the sequence if necessary, to ensure given length
		*/
		void reserve(dtime length);

		/**
			Stores the signal to a PCM-encoded WAV file
		*/
		void saveWAV(const char* filename);

		static AudioSignal* loadWAV(Environment& env, const char* fileName);

		unsigned char getChannelCount() const { return channelCount; }
		AudioSampleFormat getSampleFormat() const { return format; }
		Environment& getEnvironment() const { return env; }
	};
}
