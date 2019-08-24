package Beatmup.Geometry;

/**
 * Integer-valued rectangle
 */
public class IntRectangle {
    public int x1, y1, x2, y2;

    public IntRectangle() {
        x1 = y1 = x2 = y2 = 0;
    }

    public IntRectangle(int centerX, int centerY, int size) {
        x1 = centerX - size/2;
        y1 = centerY - size/2;
        x2 = centerX + size/2;
        y2 = centerY + size/2;
    }

    public IntRectangle(int x1, int y1, int x2, int y2) {
        this.x1 = x1;
        this.y1 = y1;
        this.x2 = x2;
        this.y2 = y2;
    }

    public int getWidth() {
        return x2-x1;
    }

    public int getHeight() {
        return y2-y1;
    }

    /**
     * Expands the rectangle so that it have a given point inside or on the boundary
     * @param x     horizontal coordinate of the given point
     * @param y     horizontal coordinate of the given point
     */
    public void expandTo(int x, int y) {
        if (x < x1)
            x1 = x;
        if (x2 < x)
            x2 = x;
        if (y < y1)
            y1 = y;
        if (y2 < y)
            y2 = y;
    }


    /**
     * Expands / contracts the rectangle
     * @param border    expansion size (if negative, contraction)
     */
    public void expand(int border) {
        x1 -= border;
        y1 -= border;
        x2 += border;
        y2 += border;
    }

    public IntRectangle scale(int x, int y) {
        return new IntRectangle(x1*x, y1*y, x2*x, y2*y);
    }

    public Rectangle scale(float x, float y) {
        return new Rectangle(x1*x, y1*y, x2*x, y2*y);
    }

    /**
     * Flips rectangle corners so that its size become positive
     */
    public void makeReal() {
        if (x2 < x1) {
            int swap = x1;
            x1 = x2;
            x2 = swap;
        }
        if (y2 < y1) {
            int swap = y1;
            y1 = y2;
            y2 = swap;
        }
    }


    public static float fit(int innerWidth, int innerHeight, int outerWidth, int outerHeight) {
        return innerWidth * outerHeight > innerHeight * outerWidth ?
                (float) outerWidth / innerWidth : (float) outerHeight / innerHeight;
    }


    @Override
    public String toString() {
        return "((" + Integer.toString(x1) + ", " + Integer.toString(y1) + "), (" + Integer.toString(x2) + ", " + Integer.toString(y2) + "))";
    }
}
