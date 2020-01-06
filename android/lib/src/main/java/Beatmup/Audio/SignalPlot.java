package Beatmup.Audio;

import Beatmup.Bitmap;
import Beatmup.Context;
import Beatmup.Geometry.IntRectangle;
import Beatmup.Imaging.Color;
import Beatmup.Task;


/**
 * Efficient audio signal magnitude graph plotting
 */
public class SignalPlot extends Task {
    private static native long newSignalPlot(Context context);
    private static native void prepareMetering(long handle);
    private native void setSignal(long handle, long signalHandle);
    private native void setBitmap(long handle, Bitmap bitmap);
    private native void setWindow(long handle, int t1, int t2, int y1, int y2, float scale);
    private native void setPlotArea(long handle, int x1, int y1, int x2, int y2);
    private native void setPalette(long handle, int bgRgba, int c1Rgba, int c2Rgba);
    private native void setChannels(long handle, int channels);

    private Signal signal;
    private Bitmap bitmap;

    public SignalPlot(Context context) {
        super(context, newSignalPlot(context));
    }

    /**
     * Sets source signal to plot.
     * @param signal    the signal to plot
     */
    public void setSignal(Signal signal) {
        this.signal = signal;
        setSignal(this.handle, signal.getHandle());
    }

    public Signal getSignal() {
        return signal;
    }

    /**
     * Sets an output bitmap the signal plot is drawn onto.
     * @param bitmap    the bitmap
     */
    public void setBitmap(Bitmap bitmap) {
        this.bitmap = bitmap;
        setBitmap(this.handle, bitmap);
    }

    public Bitmap getBitmap() {
        return bitmap;
    }


    /**
     * Specifies a rectangular area in pixels on the output bitmap the plot is drawn to.
     * @param area  the plotting area
     */
    public void setPlotArea(IntRectangle area) {
        setPlotArea(handle, area.x1, area.y1, area.x2, area.y2);
    }


    /**
     * Specifies a time/magnitude window of the signal to be plotted.
     * X coordinates are time boundaries in samples. Y coordinates are integer-valued signal
     * magnitude, e.g. -128 and + 127 for an 8-bit signal.
     * @param window    the signal window (time and magnitude)
     * @param scale     magnitude scaling factor
     */
    public void setWindow(IntRectangle window, float scale) {
        setWindow(handle, window.x1, window.x2, window.y1, window.y2, scale);
    }


    /**
     * Specifies plot colors
     * @param background    background color
     * @param color1        main plotting color
     * @param color2        second plotting color used when plotting all the channels together (see
     *                      setChannels())
     */
    public void setPalette(Color background, Color color1, Color color2) {
        setPalette(handle, background.getRgbaCode(), color1.getRgbaCode(), color2.getRgbaCode());
    }


    /**
     * Specifies which channels to plot.
     * @param channels  a channel number starting from zero to plot a specific channel; any other
     *                  value out of the range to plot all the channels together
     */
    public void setChannels(int channels) {
        setChannels(handle, channels);
    }


    /**
     * Recomputes signal dynamics lookup table allowing for an accelerated plotting.
     */
    public void prepareMetering() {
        if (signal != null)
            prepareMetering(signal.getHandle());
    }
}
