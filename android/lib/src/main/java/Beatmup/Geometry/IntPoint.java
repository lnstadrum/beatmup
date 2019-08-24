package Beatmup.Geometry;

public class IntPoint {
    public int x, y;

    public IntPoint() {
        x = y = 0;
    }

    public IntPoint(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public IntPoint(IntPoint another) {
        this.x = another.x;
        this.y = another.y;
    }

    public void set(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public void assign(IntPoint another) {
        this.x = another.x;
        this.y = another.y;
    }
}