package Beatmup.Imaging;

/**
 * Enumeration of Beatmup pixel formats
 */
public enum PixelFormat {
    SingleByte,
    TripleByte,
    QuadByte,

    SingleFloat,
    TripleFloat,
    QuadFloat,

    BinaryMask,
    QuaternaryMask,
    HexMask;

    private int[] BITS_PER_PIXEL = {8, 24, 32, 32, 96, 128, 1, 2, 4};

    public int getBitsPerPixel() {
        return BITS_PER_PIXEL[ordinal()];
    }
}