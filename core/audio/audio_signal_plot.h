#pragma once
#include "audio_signal.h"
#include "../bitmap/abstract_bitmap.h"
#include "../parallelism.h"
#include "audio_signal.h"
#include <vector>

namespace Beatmup {

    class AudioSignalPlot : public AbstractTask {
    private:
        AudioSignal* signal;
        AbstractBitmap* bitmap;
        IntRectangle outputRect;
        IntRectangle signalWindow;
        float scale;
        int channels;

        std::vector<int> values;

        struct {
            color4i bgColor, color1, color2;
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
        void setSignal(AudioSignal*);

        /**
            Sets the output bitmap
        */
        void setBitmap(AbstractBitmap*);

        /**
            Specifies a rectangular area in pixels in the output bitmap where the plot will be drawn
        */
        void setPlotArea(IntRectangle);

        /**
            Specifies a time range (X coordinate) and a magnitude range (Y coordinate scaled by `scale`) that will be plotted
            \param window		a rectangle on time-value plane containing the two ranges
            \param scale		magnitude scaling
        */
        void setWindow(IntRectangle window, float scale);

        /**
            Specifies plot colors
        */
        void setPalette(color4i bgColor, color4i color1, color4i color2);

        /**
            Specifies which channels to plot
            \param channels			a channel number (counted from 0) to plot a single channel; any number out of correct range to plot all channels
        */
        void setChannels(int channels);

        inline AbstractBitmap* getBitmap() const { return bitmap; }
        inline AudioSignal* getSignal() const { return signal; }
    };
}
