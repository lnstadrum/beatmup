package Beatmup;

import android.graphics.Bitmap.Config;
import android.util.Log;

import Beatmup.Context;
import Beatmup.Geometry.IntRectangle;
import Beatmup.Imaging.PixelFormat;


/**
 * Bitmap envelope
 */
public class Bitmap extends Beatmup.Object {
    protected Context context;

    // native methods
    private native static long newInternalBitmap(Context context, int width, int height, int pixelFormat);

    protected native static long newNativeBitmap(Context context, android.graphics.Bitmap bitmap);

    private native int getWidth(long handle);
    private native int getHeight(long handle);
    private native int getPixelFormat(long handle);

    private native void zero(long handle);
    private native void crop(long handle, long outputHandle, int x1, int y1, int x2, int y2, int outLeft, int outTop);

    protected native static void pullPixels(long handle);


    private static long createNativeBitmapEnsuringPixelFormat(Context context, android.graphics.Bitmap bitmap) throws BadPixelFormat {
        if (bitmap.getConfig() != android.graphics.Bitmap.Config.ALPHA_8  &&  bitmap.getConfig() != android.graphics.Bitmap.Config.ARGB_8888)
            throw new BadPixelFormat(bitmap.getConfig());
        return newNativeBitmap(context, bitmap);
    }


    protected Bitmap(Context context, long handle) {
        super(handle);
        this.context = context;
        context.watchBitmap(this);
    }

    
    /**
     * Creates new bitmap from Android bitmap object. NO PIXEL DATA IS COPIED.
     * @param context   Beatmup context
     * @param bitmap    source bitmap
     * @throws BadPixelFormat
     */
    protected Bitmap(Context context, android.graphics.Bitmap bitmap) throws BadPixelFormat {
        super(createNativeBitmapEnsuringPixelFormat(context, bitmap));
        this.context = context;
        context.watchBitmap(this);
    }


    /**
     * Creates new internally managed bitmap
     * @param context       Beatmup context
     * @param width         bitmap width in pixels
     * @param height        bitmap height in pixels
     * @param pixelFormat   bitmap pixel format
     */
    public Bitmap(Context context, int width, int height, PixelFormat pixelFormat) {
        super(newInternalBitmap(context, width, height, pixelFormat.ordinal()));
        this.context = context;
        context.watchBitmap(this);
    }


    /**
     * Creates a bitmap whose right bound is byte-aligned
     * @param context       Beatmup context
     * @param minWidth      minimal bitmap width in pixels; the actual one may be bigger to have the aligned right boundary
     * @param height        bitmap height in pixels
     * @param pixelFormat   bitmap pixel format
     * @return new byte-aligned bitmap
     */
    static Bitmap createByteAligned(Context context, int minWidth, int height, PixelFormat pixelFormat) {
        int pixPerByte = 8 / pixelFormat.getBitsPerPixel();
        if (pixPerByte > 1)
            minWidth = ((minWidth + pixPerByte - 1) / pixPerByte) * pixPerByte;
        return new Bitmap(context, minWidth, height, pixelFormat);
    }


    /**
     * Creates an empty bitmap of the same size and pixel format as a given bitmap
     * @param bitmap        the bitmap specifying size and pixel format
     * @return the new bitmap
     */
    static public Bitmap createEmpty(Bitmap bitmap) {
        return new Bitmap(bitmap.context, bitmap.getWidth(), bitmap.getHeight(), bitmap.getPixelFormat());
    }


    /**
     * @return bitmap width in pixels
     */
    public int getWidth() {
        return getWidth(handle);
    }


    /**
     * @return bitmap height in pixels
     */
    public int getHeight() {
        return getHeight(handle);
    }


    /**
     * @return bitmap border rectangle in pixels
     */
    public IntRectangle clientRectangle() {
        return new IntRectangle(0, 0, getWidth()-1, getHeight()-1);
    }


    /**
     * @return bitmap pixel format
     */
    public PixelFormat getPixelFormat() {
        return PixelFormat.values()[ getPixelFormat(handle) ];
    }


    /**
     * Sets all bitmap pixels to zero
     */
    public void zero() {
        zero(handle);
    }

    public static void recycle(Bitmap bitmap) {
        if (bitmap != null)
            bitmap.dispose();
    }


    /**
     * @return copy of this bitmap
     */
    public Bitmap clone() {
        return context.copyBitmap(this, getPixelFormat());
    }


    /**
     * @return copy of this bitmap with a different pixel format
     */
    public Bitmap clone(PixelFormat pixelFormat) {
        return context.copyBitmap(this, pixelFormat);
    }


    /**
     * Creates a bitmap containing a copy of a rectangular region
     * @param region        the region to copy
     * @return the new bitmap
     */
    public Bitmap copyRegion(IntRectangle region) {
        Bitmap copy = Bitmap.createByteAligned(context, region.getWidth(), region.getHeight(), getPixelFormat());
        if (copy.getWidth() != region.getWidth())
            copy.zero();
        crop(handle, copy.handle, region.x1, region.y1, region.x2, region.y2, 0, 0);
        return copy;
    }

    /**
     * Copies a rectangular area to another bitmap. The area size is equal to the target bitmap size.
     * @param bitmap    the target bitmap
     * @param left      source area top-left corner horizontal position in pixels
     * @param top       source area top-left corner vertical position in pixels
     */
    public void projectOn(Bitmap bitmap, int left, int top) {
        crop(handle, bitmap.handle, left, top, left+bitmap.getWidth(), top+bitmap.getHeight(), 0, 0);
    }

    /**
     * Transfers pixel data from GPU to CPU
     */
    public void pullPixels() {
        pullPixels(handle);
    }

    /**
     * CoreException thrown when a native bitmap has incompatible pixel format
     */
    public static class BadPixelFormat extends IllegalArgumentException {
        public BadPixelFormat(Config config) {
            super("Pixel format not supported: " + (config == null ? "config is null" : config.toString()));
                // config may be null for some bitmaps
        }
    }
}