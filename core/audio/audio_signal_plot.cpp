#include "audio_signal_plot.h"
#include "../color/colors.h"
#include "../bitmap/bitmap_access.h"
#include "../bitmap/processing.h"
#include <algorithm>
#include <vector>

using namespace Beatmup;


template<class out_t> class DrawBars3 {
public:

    /**
        Plots three vertical bars
        \param bitmap		a bitmap
        \param x		the horizontal coordinate
        \param y 		the bar top vertical coordinate
        \param y1		end of first bar (excluded)
        \param y2		end of second bar (included)
        \param y3		end of third bar (included)
        \param color1	color of 1st and 3rd bars
        \param color2	color of 2nd bar
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
        \param ptr		a bitmap
        \param x		the horizontal coordinate
        \param y 		the bar top vertical coordinate
        \param y1		end of first bar (excluded)
        \param y2		end of second bar (excluded)
        \param y3		end of third bar (included)
        \param y4		end of fourth bar (included)
        \param y5		end of fifth bar (included)
        \param color1	color of 1st and 5th bars
        \param color2	color of 2nd and 4th bars
        \param color3	color of 3rd bar
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


void AudioSignalPlot::getPlot(TaskThread& thread, std::vector<int>& data, int& left, int& right) {
    const dtime
        startTime = signalWindow.a.x + thread.currentThread() * signalWindow.width() / thread.totalThreads(),
        stopTime  = signalWindow.a.x + (thread.currentThread() + 1) * signalWindow.width() / thread.totalThreads(),
        length = stopTime - startTime;
    left = thread.currentThread() * outputRect.width() / thread.totalThreads();
    right = (thread.currentThread() + 1) * outputRect.width() / thread.totalThreads();

    const int width = right - left;

    const float
        mag = (float)signalWindow.height(),
        heightMag = (outputRect.height() - 1) / mag;

    // measure!
    const int count = width * signal->getChannelCount();
    typedef sample16 sample;
    sample *min = new sample[count], *max = new sample[count];
    AudioSignal::Meter ptr(*signal, startTime);
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


AudioSignalPlot::AudioSignalPlot():
    signal(NULL), bitmap(NULL), outputRect(0,0,100,100), signalWindow(0,-32768,100,32767), scale(1.0f), channels(-1)
    //fixme: set reasonable values
{
    palette.bgColor = Colors::White;
    palette.color1 = Colors::DarkSeaGreen1;
    palette.color2 = Colors::DarkSeaGreen2;
}


bool AudioSignalPlot::process(TaskThread& thread) {
    std::vector<int> yyy;
    int x0, x1;
    getPlot(thread, yyy, x0, x1);
    thread.synchronize();
    auto y = yyy.begin();
    // single channel
    if (0 <= channels && channels < signal->getChannelCount())
        for (int x = x0; x < x1 && !thread.isTaskAborted(); x++) {
            int y0 = *y++, y1 = *y++;
            BitmapProcessing::write<DrawBars3>(*bitmap, x, outputRect.a.y, y0, y1, outputRect.b.y, palette.bgColor, palette.color1);
        }
    // all channels
    else
        for (int x = x0; x < x1 && !thread.isTaskAborted(); x++) {
            int y0 = *y++, y1 = *y++, y2 = *y++, y3 = *y++;
            BitmapProcessing::write<DrawBars5>(
                *bitmap, x, outputRect.a.y, y0, y1, y2, y3, outputRect.b.y,
                palette.bgColor, palette.color1, palette.color2
            );
        }
    return true;
}


void AudioSignalPlot::beforeProcessing(ThreadIndex threadCount, GraphicPipeline* gpu) {
    NullTaskInput::check(signal, "input signal");
    NullTaskInput::check(bitmap, "output bitmap");
    bitmap->lockContent(PixelFlow::CpuWrite);
    outputRect.normalize();
    signalWindow.normalize();
}


void AudioSignalPlot::afterProcessing(ThreadIndex, GraphicPipeline*, bool) {
    bitmap->unlockContent(PixelFlow::CpuWrite);
}


AbstractTask::ExecutionTarget AudioSignalPlot::getExecutionTarget() const {
    return ExecutionTarget::doNotUseGPU;
}


ThreadIndex AudioSignalPlot::maxAllowedThreads() const {
    BEATMUP_ASSERT_DEBUG(bitmap != nullptr);
    return validThreadCount(bitmap->getWidth());
}


void AudioSignalPlot::setSignal(AudioSignal* aSignal) {
    signal = aSignal;
}


void AudioSignalPlot::setBitmap(AbstractBitmap* aBitmap) {
    bitmap = aBitmap;
}


void AudioSignalPlot::setPlotArea(IntRectangle aRectangle) {
    outputRect = aRectangle;
}


void AudioSignalPlot::setWindow(IntRectangle aWindow, float aScale) {
    signalWindow = aWindow;
    scale = aScale;
}


void AudioSignalPlot::setPalette(color4i bgColor, color4i color1, color4i color2) {
    palette.bgColor = bgColor;
    palette.color1 = color1;
    palette.color2 = color2;
}


void AudioSignalPlot::setChannels(int channels) {
    this->channels = channels;
}
