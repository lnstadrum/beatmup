package Beatmup.Imaging;

public class FloatColor {
    public float r, g, b, a;

    public FloatColor() {
        r = g = b = a = 0;
    }

    public FloatColor(float r, float g, float b, float a) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
        // This constructor is called by JNI, do not remove it.
    }


    public Color getIntColor() {
        return new Color(Math.round(r * 255), Math.round(g * 255), Math.round(b * 255), Math.round(a * 255));
    }
}
