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

#include "signal_plot.h"
#include "../color/constants.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/processing.h"
#include <algorithm>
#include <vector>

using namespace Beatmup;
using namespace Audio;

namespace Kernels {
    template<class out_t> class DrawBars3 {
    public:

        /**
            Plots three vertical bars
            \param bitmap	    A bitmap to draw onto
            \param x		    The horizontal coordinate
            \param y 		    The bar top vertical coordinate
            \param y1		    End of first bar (excluded)
            \param y2		    End of second bar (included)
            \param y3		    End of third bar (included)
            \param color1	    Color of 1st and 3rd bars
            \param color2	    Color of 2nd bar
        */
        static void process(AbstractBitmap& bitmap, int x, int y, int y1, int y2, int y3, const color4i& color1, const color4i& color2) {
            out_t ptr(bitmap, x, y);
            out_t ref = ptr;
            ref.goTo(x, y1);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color1);
                ptr += ptr.getWidth();
            }
            ref.goTo(x, y2);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color2);
                ptr += ptr.getWidth();
            }
            ptr = pixint4::fromColor(color2);
            ptr += ptr.getWidth();
            ref.goTo(x, y3);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color1);
                ptr += ptr.getWidth();
            }
            ptr = pixint4::fromColor(color1);
        }
    };


    template<typename out_t> class DrawBars5 {
    public:

        /**
            Plots five vertical bars
            \param bitmap       A bitmap to draw onto
            \param x		    The horizontal coordinate
            \param y 		    The bar top vertical coordinate
            \param y1		    End of first bar (excluded)
            \param y2		    End of second bar (excluded)
            \param y3		    End of third bar (included)
            \param y4		    End of fourth bar (included)
            \param y5		    End of fifth bar (included)
            \param color1	    Color of 1st and 5th bars
            \param color2	    Color of 2nd and 4th bars
            \param color3	    Color of 3rd bar
        */
        static void process(
            AbstractBitmap& bitmap,
            int x, int y, int y1, int y2, int y3, int y4, int y5,
            const color4i& color1, const color4i& color2, const color4i& color3
        ) {
            out_t ptr(bitmap, x, y);
            out_t ref = ptr;
            ref.goTo(x, y1);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color1);
                ptr += ptr.getWidth();
            }
            ref.goTo(x, y2);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color2);
                ptr += ptr.getWidth();
            }
            ref.goTo(x, y3);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color3);
                ptr += ptr.getWidth();
            }
            ptr = pixint4::fromColor(color3);
            ptr += ptr.getWidth();
            ref.goTo(x, y4);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color2);
                ptr += ptr.getWidth();
            }
            if (y3 < y4) {
                ptr = pixint4::fromColor(color2);
                ptr += ptr.getWidth();
            }
            ref.goTo(x, y5);
            while (ptr < ref) {
                ptr = pixint4::fromColor(color1);
                ptr += ptr.getWidth();
            }
            ptr = pixint4::fromColor(color1);
        }
    };
}


void SignalPlot::getPlot(TaskThread& thread, std::vector<int>& data, int& left, int& right) {
    const dtime
        startTime = signalWindow.a.x + thread.currentThread() * signalWindow.width() / thread.numThreads(),
        stopTime  = signalWindow.a.x + (thread.currentThread() + 1) * signalWindow.width() / thread.numThreads(),
        length = stopTime - startTime;
    left = thread.currentThread() * outputRect.width() / thread.numThreads();
    right = (thread.currentThread() + 1) * outputRect.width() / thread.numThreads();

    const int width = right - left;

    const float
        mag = (float)signalWindow.height(),
        heightMag = (outputRect.height() - 1) / mag;

    // measure!
    const int count = width * signal->getChannelCount();
    typedef sample16 sample;
    sample *min = new sample[count], *max = new sample[count];
    Signal::Meter ptr(*signal, startTime);
    ptr.measure<sample>(length, width, min, max);

    // compute bin heights in function of the channel mode
    // single channel
    if (0 <= channels && channels < signal->getChannelCount()) {
        data.reserve(2 * width);
        sample* pMin = min + channels, *pMax = max + channels;
        for (int x = 0; x < width; x++) {
            data.push_back( outputRect.b.y - (int)std::min(std::max(.0f, pMin->x * scale - signalWindow.a.y), mag) * heightMag );
            data.push_back( outputRect.b.y - (int)std::min(std::max(.0f, pMax->x * scale - signalWindow.a.y), mag) * heightMag );
            pMin += channels;
            pMax += channels;
        }
    }
    // combined channels: minmin --- minmax *** maxmin --- maxmax
    else {
        data.reserve(4 * width);
        sample* pMin = min, *pMax = max;
        for (int x = 0; x < width; x++) {
            sample ymm = *pMin++, ymM = ymm, yMm = *pMax++, yMM = yMm;
            for (int ch = 1; ch < signal->getChannelCount(); ch++) {
                ymm = std::min(ymm, *pMin);
                ymM = std::max(ymM, *pMin++);
                yMm = std::min(yMm, *pMax);
                yMM = std::max(yMM, *pMax++);
            }
            if (yMm < ymM) {
                sample _ = ymM;
                ymM = yMm;
                yMm = _;
            }
            data.push_back(outputRect.b.y - (int)std::min(std::max(.0f, yMM.x * scale - signalWindow.a.y), mag) * heightMag);
            data.push_back(outputRect.b.y - (int)std::min(std::max(.0f, yMm.x * scale - signalWindow.a.y), mag) * heightMag);
            data.push_back(outputRect.b.y - (int)std::min(std::max(.0f, ymM.x * scale - signalWindow.a.y), mag) * heightMag);
            data.push_back(outputRect.b.y - (int)std::min(std::max(.0f, ymm.x * scale - signalWindow.a.y), mag) * heightMag);
        }
    }
    delete[] min;
    delete[] max;
}


SignalPlot::SignalPlot():
    signal(NULL), bitmap(NULL), outputRect(0,0,100,100), signalWindow(0,-32768,100,32767), scale(1.0f), channels(-1)
    //fixme: set reasonable values
{
    palette.bgColor = Color::WHITE;
    palette.color1 = Color::DARK_SEA_GREEN1;
    palette.color2 = Color::DARK_SEA_GREEN2;
}


bool SignalPlot::process(TaskThread& thread) {
    std::vector<int> yyy;
    int x0, x1;
    getPlot(thread, yyy, x0, x1);
    thread.synchronize();
    auto y = yyy.begin();
    // single channel
    if (0 <= channels && channels < signal->getChannelCount())
        for (int x = x0; x < x1 && !thread.isTaskAborted(); x++) {
            int y0 = *y++, y1 = *y++;
            BitmapProcessing::write<Kernels::DrawBars3>(*bitmap, x, outputRect.a.y, y0, y1, outputRect.b.y, palette.bgColor, palette.color1);
        }
    // all channels
    else
        for (int x = x0; x < x1 && !thread.isTaskAborted(); x++) {
            int y0 = *y++, y1 = *y++, y2 = *y++, y3 = *y++;
            BitmapProcessing::write<Kernels::DrawBars5>(
                *bitmap, x, outputRect.a.y, y0, y1, y2, y3, outputRect.b.y,
                palette.bgColor, palette.color1, palette.color2
            );
        }
    return true;
}


void SignalPlot::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    NullTaskInput::check(signal, "input signal");
    NullTaskInput::check(bitmap, "output bitmap");
    writeLock(gpu, bitmap,  ProcessingTarget::CPU);
    outputRect.normalize();
    signalWindow.normalize();
}


void SignalPlot::afterProcessing(ThreadIndex, GraphicPipeline*, bool) {
    unlock(bitmap);
}


AbstractTask::TaskDeviceRequirement SignalPlot::getUsedDevices() const {
    return TaskDeviceRequirement::CPU_ONLY;
}


ThreadIndex SignalPlot::getMaxThreads() const {
    BEATMUP_ASSERT_DEBUG(bitmap != nullptr);
    return validThreadCount(bitmap->getWidth());
}


void SignalPlot::setSignal(Signal* signal) {
    this->signal = signal;
}


void SignalPlot::setBitmap(AbstractBitmap* bitmap) {
    this->bitmap = bitmap;
}


void SignalPlot::setPlotArea(IntRectangle rectangle) {
    this->outputRect = rectangle;
}


void SignalPlot::setWindow(IntRectangle window, float scale) {
    this->signalWindow = window;
    this->scale = scale;
}


void SignalPlot::setPalette(color4i bgColor, color4i color1, color4i color2) {
    palette.bgColor = bgColor;
    palette.color1 = color1;
    palette.color2 = color2;
}


void SignalPlot::setChannels(int channels) {
    this->channels = channels;
}
