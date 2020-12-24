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

#pragma once
#include "../bitmap/abstract_bitmap.h"
#include "../parallelism.h"
#include "signal.h"
#include <vector>

namespace Beatmup {
namespace Audio {

    /**
        AbstractTask drawing amplitude graph of a given Signal in a bitmap.
    */
    class SignalPlot : public AbstractTask, private BitmapContentLock {
    private:
        Signal* signal;
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
        virtual void beforeProcessing(ThreadIndex, ProcessingTarget target, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, GraphicPipeline*, bool);
        virtual TaskDeviceRequirement getUsedDevices() const;
        virtual ThreadIndex getMaxThreads() const;

    public:
        SignalPlot();

        /**
            Sets the input signal to plot.
        */
        void setSignal(Signal*);

        /**
            Sets the output bitmap.
        */
        void setBitmap(AbstractBitmap*);

        /**
            Specifies a rectangular area in pixels in the output bitmap where the plot will be drawn.
        */
        void setPlotArea(IntRectangle);

        /**
            Specifies a time range (X coordinate) and a magnitude range (Y coordinate scaled by `scale`) that will be plotted.
            \param window		a rectangle in time-value plane containing the two ranges
            \param scale		magnitude scaling factor
        */
        void setWindow(IntRectangle window, float scale);

        /**
            Specifies plot colors.
            \param bgColor      background color
            \param color1       main plotting color
            \param color2       second plotting color used when plotting all the channels together (see setChannels())
        */
        void setPalette(color4i bgColor, color4i color1, color4i color2);

        /**
            Specifies which channels to plot
            \param channels			a channel number (counted from 0) to plot a single channel; any number out of correct range to plot all channels
        */
        void setChannels(int channels);

        inline AbstractBitmap* getBitmap() const { return bitmap; }
        inline Signal* getSignal() const { return signal; }
    };

}
}
