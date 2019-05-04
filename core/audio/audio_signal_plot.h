#pragma once
#include "audio_signal.h"
#include "../bitmap/abstract_bitmap.h"
#include "../parallelism.h"
#include "audio_signal.h"
#include "../bitmap/pixel_arithmetic.h"
#include <vector>

namespace Beatmup {

	class AudioSignalPlot : public AbstractTask {
	private:
		AudioSignal* signal;
		AbstractBitmap* output;
		IntRectangle outputRect;
		IntRectangle signalWindow;
		float scale;
		int channels;
		
		std::vector<int> values;
		
		struct {
			pixint4 bgColor, color1, color2;
		} palette;
		
		/**
			\internal
			Computes plot data for a given thread.
		*/
		void getPlot(TaskThread& thread, std::vector<int>& data, int& left, int& right);

	protected:
		virtual bool process(TaskThread& thread);
		virtual void beforeProcessing(ThreadIndex, GraphicPipeline*);
		virtual void afterProcessing(ThreadIndex, bool);
		virtual ExecutionTarget getExecutionTarget() const;
		virtual ThreadIndex maxAllowedThreads() const;

	public:
		AudioSignalPlot();

		/**
			Sets the input signal to plot
		*/
		void setSignal(AudioSignal&);
		
		/**
			Sets the output bitmap
		*/
		void setOutput(AbstractBitmap&);
		
		/**
			Specifies a rectangular area in pixels in the output bitmap where the plot will be drawn
		*/
		void setOutputArea(IntRectangle);

		/**
			Specifies a time range (X coordinate) and a magnitude range (Y coordinate scaled by `scale`) that will be plotted
			\param window		a rectangle on time-value plane containing the two ranges
			\param scale		magnitude scaling
		*/
		void setSignalWindow(IntRectangle window, float scale);

		/**
			Specifies plot colors
		*/
		void setPalette(pixint4 bgColor, pixint4 color1, pixint4 color2);

		/**
			Specifies which channels to plot
			\param channels			a channel number (counted from 0) to plot a single channel; any number out of correct range to plot all channels
		*/
		void setChannels(int channels);

		inline AbstractBitmap* getOutput() const { return output; }
		inline AudioSignal* getSignal() const { return signal; }
	};
}