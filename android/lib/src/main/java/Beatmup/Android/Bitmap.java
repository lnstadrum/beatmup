package Beatmup.Android;

import android.graphics.BitmapFactory;

import java.io.FileDescriptor;
import java.io.InputStream;

import Beatmup.Context;

/**
 * Android bitmap wrapping with no risk of memory leak
 */
public class Bitmap extends Beatmup.Bitmap {

    private android.graphics.Bitmap source;     //!< wrapped Android bitmap

    /**
     * Creates new bitmap from Android bitmap object; NO PIXEL DATA IS COPIED
     * @param context   Beatmup context
     * @param bitmap    source bitmap
     */
    private Bitmap(Context context, android.graphics.Bitmap bitmap) {
        super(context, bitmap);
        source = bitmap;
    }

    /**
     * Creates new bitmap from a source Android bitmap converting the data into appropriate pixel format if necessary
     * @param context   the context handling the new bitmap
     * @param bitmap    the source; may be recycled
     * @return new bitmap
     */
    private static Bitmap createEnsuringPixelFormat(Context context, android.graphics.Bitmap bitmap) {
        if (bitmap.getConfig() != android.graphics.Bitmap.Config.ALPHA_8  &&  bitmap.getConfig() != android.graphics.Bitmap.Config.ARGB_8888) {
            android.graphics.Bitmap copy = bitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, true);
            bitmap.recycle();
            return new Bitmap(context, copy);
        } else
            return new Bitmap(context, bitmap);
    }


    /**
     * Decodes a bitmap from stream
     * @param context       a Beatmup context
     * @param inputStream   the stream to decode
     * @return bitmap containing the decoded stream content
     */
    public static Bitmap decodeStream(Context context, InputStream inputStream) throws OutOfMemoryError {
        android.graphics.Bitmap bitmap = BitmapFactory.decodeStream(inputStream);
        if (bitmap == null)
            return null;
        return createEnsuringPixelFormat(context, bitmap);
    }

    /**
     * Loads a bitmap from file. If the pixel format is not supported, this function tries to convert the source data to a 32-bit color bitmap
     * @param context       a Beatmup context
     * @param path          path to the file to decode
     * @param options       bitmap loader options
     * @return bitmap from the file
     */
    public static Bitmap decodeFile(Context context, String path, BitmapFactory.Options options) throws OutOfMemoryError {
        android.graphics.Bitmap bitmap = BitmapFactory.decodeFile(path, options);
        if (bitmap == null)
            return null;
        return createEnsuringPixelFormat(context, bitmap);
    }


    /**
     * Loads a bitmap from file
     * @param context       a Beatmup context
     * @param file          a file descriptor of the content to decode
     * @param outPadding    outPadding returned by BitmapFactory.decodeFileDescriptor
     * @return bitmap from the file
     */
    public static Bitmap decodeFileDescriptor(
            Context context,
            FileDescriptor file,
            android.graphics.Rect outPadding,
            BitmapFactory.Options options
    ) throws OutOfMemoryError
    {
        android.graphics.Bitmap bitmap = BitmapFactory.decodeFileDescriptor(file, outPadding, options);
        if (bitmap == null)
            return null;
        return createEnsuringPixelFormat(context, bitmap);
    }


    /**
     * Creates empty ARGB bitmap of specified size
     * @param context   a Beatmup context
     * @param width     width of the new bitmap in pixels
     * @param height    height of the new bitmap in pixels
     * @return the new bitmap
     */
    public static Bitmap createColorBitmap(Context context, int width, int height) {
        return new Bitmap(context, android.graphics.Bitmap.createBitmap(width, height, android.graphics.Bitmap.Config.ARGB_8888));
    }

    /**
     * Creates empty grayscale bitmap of specified size
     * @param context   a Beatmup context
     * @param width     width of the new bitmap in pixels
     * @param height    height of the new bitmap in pixels
     * @return the new bitmap
     */
    public static Bitmap createGrayscaleBitmap(Context context, int width, int height) {
        return new Bitmap(context, android.graphics.Bitmap.createBitmap(width, height, android.graphics.Bitmap.Config.ALPHA_8));
    }

    public android.graphics.Bitmap getBitmap() {
        return source;
    }


    @Override
    public synchronized void dispose() {
        if (source != null) {
            source.recycle();
            source = null;
        }
        super.dispose();
    }


    /**
     * @return copy of this bitmap
     */
    public Bitmap clone() {
        return new Bitmap(context, source.copy(source.getConfig(), true));
    }
}
