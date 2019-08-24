package Beatmup.Geometry;

/**
 * A 2D point
 */
public class Point {
    public float x, y;

    public Point() {
        x = y = 0;
    }

    public Point(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public Point(Point another) {
        this.x = another.x;
        this.y = another.y;
    }

    public void set(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public void assign(Point another) {
        this.x = another.x;
        this.y = another.y;
    }
}
