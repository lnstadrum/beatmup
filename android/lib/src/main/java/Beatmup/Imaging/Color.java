package Beatmup.Imaging;

/**
 * Integer RGBA color
 */
public class Color {
    public int r, g, b, a;

    public Color() {
        r = g = b = a = 0;
    }

    public Color(int r, int g, int b, int a) {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
        // This constructor is called by JNI, do not remove it.
    }

    public Color(int code) {
        b = code % 256;
        g = (code >> 8) % 256;
        r = (code >> 16) % 256;
        a = (code >> 24) % 256;
    }


    public int getCode() {
        return b | g << 8 | r << 16 | a << 24;
    }


    public static Color parseString(String expr) {
        // hex notation
        if (expr.matches("(\\d|[a-fA-F]){6}")) {
            Color c = new Color(Integer.parseInt(expr, 16));
            c.a = 255;
            return c;
        }
        if (expr.matches("(\\d|[a-fA-F]){8}"))
            return new Color(Integer.parseInt(expr, 16));
        // comma split notation
        String vals[] = expr.split(",");
        if (vals.length != 3 && vals.length != 4)
            throw new IllegalArgumentException("Cannot parse color from expression '" +expr+ "'");
        return new Color(
            Integer.parseInt(vals[0]),
            Integer.parseInt(vals[1]),
            Integer.parseInt(vals[2]),
            vals.length > 3 ? Integer.parseInt(vals[3]) : 255
        );
    }


    public Color scale(float factorRGB, float factorAlpha) {
        return new Color(
                Math.round(r * factorRGB),
                Math.round(g * factorRGB),
                Math.round(b * factorRGB),
                Math.round(a * factorAlpha)
        );
    }
    
    
    public static Color byHue(float hueDegrees) {
        final double
            SQRT3 = 1.732050807568877,
            hue = Math.toRadians(hueDegrees),
            r = Math.max(0, (2 * Math.cos(hue) + 1) / 3),
            g = Math.max(0, (SQRT3 * Math.sin(hue) - Math.cos(hue) + 1) / 3),
            b = Math.max(0, (1 - SQRT3 * Math.sin(hue) - Math.cos(hue)) / 3),
            norm = Math.max(r, Math.max(g, b));
        return new Color(
            (int) Math.floor(255 * r / norm),
            (int) Math.floor(255 * g / norm),
            (int) Math.floor(255 * b / norm),
            255
        );
    }


    public static final Color
            TRANSPARENT_BLACK = new Color(0, 0, 0, 0),
            BLACK       = new Color(0, 0, 0, 255),
            WHITE       = new Color(255, 255, 255, 255),
            RED         = new Color(255, 0, 0, 255),
            GREEN       = new Color(0, 255, 0, 255),
            BLUE        = new Color(0, 0, 255, 255),
            YELLOW      = new Color(255, 255, 0, 255),
            PURPLE      = new Color(255, 0, 255, 255),
            ORANGE      = new Color(255, 127, 0, 255),
            GRAY        = new Color(127, 127, 127, 255);
}