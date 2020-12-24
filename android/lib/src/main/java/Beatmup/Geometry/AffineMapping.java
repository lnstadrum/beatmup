/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package Beatmup.Geometry;

import android.graphics.Matrix;

/**
 * A 2x2 matrix together with a translation vector
 */
public class AffineMapping {
    public float
        a11,a12,a21,a22,            //!< matrix coefficients
        x,y;                        //!< point coordinates the point (0,0) is mapped to


    public AffineMapping() {
        a11 = 1; a21 = 0; x = 0;
        a12 = 0; a22 = 1; y = 0;
        // This constructor is called by JNI, do not remove it.
    }

    public AffineMapping(AffineMapping mapping) {
        a11 = mapping.a11;
        a12 = mapping.a12;
        a21 = mapping.a21;
        a22 = mapping.a22;
        x = mapping.x;
        y = mapping.y;
    }

    /**
     * Creates a mapping that maps the unit square to given rectangle.
     * @param rectangle the target rectangle
     */
    public AffineMapping(Rectangle rectangle) {
        x = rectangle.x1;
        y = rectangle.y1;
        a11 = rectangle.getWidth();
        a22 = rectangle.getHeight();
        a12 = a21 = 0;
    }


    @Override
    public String toString() {
        return
            Float.toString(a11) +";"+
            Float.toString(a12) +";"+
            Float.toString(x)   +";"+
            Float.toString(a21) +";"+
            Float.toString(a22) +";"+
            Float.toString(y);
    }


    /**
     * Retrieves the mapping from a string expression (typically obtained with AffineMapping.toString()).
     * @param expr      the expression to parse, row-wise enumeration of matrix elements with ';' as separator
     */
    public void fromString(String expr) {
        String vals[] = expr.split(";");
        if (vals.length != 6)
            throw new IllegalArgumentException("Affine mapping can not be parsed from expression '" +expr+ "'");
        a11 = Float.parseFloat(vals[0]);
        a12 = Float.parseFloat(vals[1]);
        x = Float.parseFloat(vals[2]);
        a21 = Float.parseFloat(vals[3]);
        a22 = Float.parseFloat(vals[4]);
        y = Float.parseFloat(vals[5]);
    }


    /**
     * Parses string and returns an affine mapping.
     * @param expr the expression to parse
     * @return parsed mapping
     */
    public static AffineMapping parse(String expr) {
        AffineMapping mapping = new AffineMapping();
        mapping.fromString(expr);
        return mapping;
    }


    /**
     * Assigns a mapping to the current mapping.
     * @param mapping   The mapping to assign to the current mapping
     */
    public void assign(AffineMapping mapping) {
        x = mapping.x;
        y = mapping.y;
        a11 = mapping.a11;
        a12 = mapping.a12;
        a21 = mapping.a21;
        a22 = mapping.a22;
    }


    /**
     * Maps a point (horizontal coordinate is computed only).
     * @param X the input point horizontal coordinate
     * @param Y the input point vertical coordinate
     * @return horizontal coordinate of the transformed point
     */
    public float mapX(float X, float Y) {
        return a11 * X + a12 * Y + this.x;
    }


    /**
     * Maps a point (vertical coordinate is computed only).
     * @param X the input point horizontal coordinate
     * @param Y the input point vertical coordinate
     * @return vertical coordinate of the transformed point
     */
    public float mapY(float X, float Y) {
        return a21 * X + a22 * Y + this.y;
    }
    public Point map(float X, float Y) {
        return new Point(mapX(X,Y), mapY(X,Y));
    }
    public Point map(Point point) {
        return new Point(mapX(point.x, point.y), mapY(point.x, point.y));
    }


    /**
     * Maps a set of points.
     * @param in        input array of point coordinates pairs (x,y)
     * @return output array of mapped points coordinates.
     */
     public float[] mapPoints(float[] in) {
         float[] out = new float[in.length];
         for (int i = 0; i < in.length/2; i++) {
             final float X = in[2*i], Y = in[2*i+1];
             out[2*i  ] = mapX(X, Y);
             out[2*i+1] = mapY(X, Y);
         }
         return out;
    }


    /**
     * Maps a set of points.
     * @param in        input array of point coordinates pairs (x,y)
     * @return output array of mapped points coordinates.
     */
    public float[] mapPoints(int[] in) {
        float[] out = new float[in.length];
        for (int i = 0; i < in.length/2; i++) {
            final float X = in[2*i], Y = in[2*i+1];
            out[2*i  ] = mapX(X, Y);
            out[2*i+1] = mapY(X, Y);
        }
        return out;
    }


    /**
     * Specifies the translation component of the mapping through center point.
     * @param cx    horizontal coordinate of the new center
     * @param cy    vertical coordinate of the new center
     */
    public AffineMapping setCenterPosition(float cx, float cy) {
        x += cx - (x + (a11 + a12)*0.5f);
        y += cy - (y + (a21 + a22)*0.5f);
        return this;
    }


    /**
     * Resets the mapping to identity.
     * @return the mapping itself
     */
    public AffineMapping setIdentity() {
        a11 = a22 = 1.0f;
        a12 = a21 = x = y = 0.0f;
        return this;
    }


    /**
     * Initializes mapping from Android matrix.
     * @param matrix    the matrix to initialize from
     */
    public AffineMapping setFromMatrix(Matrix matrix) {
        float pts[] = {0, 0,  1, 0,  0, 1};
        matrix.mapPoints(pts);
        x = pts[0];
        y = pts[1];
        a11 = pts[2] - x;
        a21 = pts[3] - y;
        a12 = pts[4] - x;
        a22 = pts[5] - y;
        return this;
    }


    /**
     * Translates the mapping in its domain.
     * @param x     horizontal translation
     * @param y     vertical translation
     * @return the mapping itself
     */
    public AffineMapping translate(float x, float y) {
        this.x += x;
        this.y += y;
        return this;
    }


    /**
     * Sets translation values.
     * @param x     horizontal translation
     * @param y     vertical translation
     * @return the mapping itself
     */
    public AffineMapping setTranslation(float x, float y) {
        this.x = x;
        this.y = y;
        return this;
    }


    /**
     * Scales the mapping around (0,0) point.
     * @param factor    scale factor
     */
    public AffineMapping scale(float factor) {
        a11 *= factor;
        a12 *= factor;
        a21 *= factor;
        a22 *= factor;
        return this;
    }


    /**
     * Scales the mapping keeping (x,y) point at the same place (in source coordinates).
     * @param x         fixed point horizontal coordinate
     * @param y         fixed point vertical coordinate
     * @param factorX   horizontal scaling factor
     * @param factorY   vertical scaling factor
     */
    public AffineMapping scaleAround(float x, float y, float factorX, float factorY) {
        final float
                x0 = a11 * x + a12 * y,
                y0 = a21 * x + a22 * y;
        a11 *= factorX;
        a12 *= factorY;
        a21 *= factorX;
        a22 *= factorY;
        this.x += x0 - (a11 * x + a12 * y);
        this.y += y0 - (a21 * x + a22 * y);
        return this;
    }

    public AffineMapping scaleAround(float x, float y, float factor) {
        return scaleAround(x, y, factor, factor);
    }


    /**
     * Rotates the mapping around (0,0) point.
     * @param angleDegrees      rotation angle in degrees
     */
    public AffineMapping rotate(float angleDegrees) {
        float
            rad = (float)(angleDegrees * Math.PI /180),
            s = (float)Math.sin(rad),
            c = (float)Math.cos(rad),
            _11 = +c * a11 + s * a12,
            _21 = +c * a21 + s * a22;
        a12 = -s * a11 + c * a12;
        a22 = -s * a21 + c * a22;
        a11 = _11;
        a21 = _21;
        return this;
    }

    public AffineMapping rotateAround(float x, float y, float angleDegrees) {
        final float
                x0 = a11 * x + a12 * y,
                y0 = a21 * x + a22 * y;
        rotate(angleDegrees);
        this.x += x0 - (a11 * x + a12 * y);
        this.y += y0 - (a21 * x + a22 * y);
        return this;
    }

    /**
     * Skew transform (first along X axis and then along Y axis).
     * @param angleXDegrees      angle in degrees for X-skew
     * @param angleYDegrees      angle in degrees for Y-skew
     */
    public AffineMapping skew(float angleXDegrees, float angleYDegrees) {
        final float
            tx = (float)Math.tan(Math.toRadians(angleXDegrees)), ty = (float)Math.tan(Math.toRadians(angleYDegrees)),
            _11 = a11 * (1 + tx*ty) + a12 * ty,
            _21 = a21 * (1 + tx*ty) + a22 * ty,
            _12 = a11 * tx + a12;
        a22 = a21 * tx + a22;
        a11 = _11;
        a12 = _12;
        a21 = _21;
        return this;
    }


    public AffineMapping skewAround(float x, float y, float angleXDegrees, float angleYDegrees) {
        final float
                x0 = a11 * x + a12 * y,
                y0 = a21 * x + a22 * y;
        skew(angleXDegrees, angleYDegrees);
        this.x += x0 - (a11 * x + a12 * y);
        this.y += y0 - (a21 * x + a22 * y);
        return this;
    }


    public AffineMapping flipAround(float x, float y, boolean horiz, boolean vertic) {
        if (!horiz && !vertic)
            return this;
        final float
                x0 = a11 * x + a12 * y,
                y0 = a21 * x + a22 * y;
        if (horiz) {
            a11 = - a11;
            a21 = - a21;
        }
        if (vertic) {
            a12 = - a12;
            a22 = - a22;
        }
        this.x += x0 - (a11 * x + a12 * y);
        this.y += y0 - (a21 * x + a22 * y);
        return this;
    }

    /**
     * Computes composition of this affine mapping with another one.
     * @param mapping       the mapping to compose with
     * @return composed mapping; its application is equivalent to apply first `mapping` and then `this`
     */
    public AffineMapping compose(AffineMapping mapping) {
        AffineMapping newbie = new AffineMapping();
        newbie.x = x + a11*mapping.x + a12*mapping.y;
        newbie.y = y + a21*mapping.x + a22*mapping.y;
        newbie.a11 = a11*mapping.a11 + a12*mapping.a21;
        newbie.a12 = a11*mapping.a12 + a12*mapping.a22;
        newbie.a21 = a21*mapping.a11 + a22*mapping.a21;
        newbie.a22 = a21*mapping.a12 + a22*mapping.a22;
        return newbie;
    }

    /**
     * Computes mappings composition `L` * `R` and assigns it to `target`.
     * @param target    a mapping to assign the composition to
     * @param L      left term of the composition
     * @param R      right term of the composition
     */
    public static void compose(AffineMapping target, AffineMapping L, AffineMapping R) {
        float _x = L.x + L.a11*R.x + L.a12*R.y;
        target.y = L.y + L.a21*R.x + L.a22*R.y;
        target.x = _x;
        float
                _a11 = L.a11*R.a11 + L.a12*R.a21,
                _a12 = L.a11*R.a12 + L.a12*R.a22,
                _a21 = L.a21*R.a11 + L.a22*R.a21;
        target.a22 = L.a21*R.a12 + L.a22*R.a22;
        target.a11 = _a11;
        target.a12 = _a12;
        target.a21 = _a21;
    }

    public static AffineMapping composeMultiple(AffineMapping... terms) {
        if (terms.length == 0)
            return null;
        AffineMapping result = new AffineMapping();
        result.assign(terms[0]);
        for (int i = 1; i < terms.length; i++)
            compose(result, result, terms[i]);
        return result;
    }


    public float det() {
        return a11*a22 - a12*a21;
    }


    public float getScale() {
        return (float) Math.sqrt(a11*a22 - a12*a21);
    }


    public float getOrientationDegrees() {
        return (float) ( Math.atan2(a11, a12) *180/ Math.PI );
    }


    public AffineMapping getInverse() {
        AffineMapping inverse = new AffineMapping();
        if (getInverse(inverse))
            return inverse;
        return null;
    }

    public boolean getInverse(AffineMapping inverse) {
        float det = a11*a22 - a12*a21;
        if (det == 0)
            return false;
        float
            _a11 = a22 / det,
            _a22 = a11 / det,
            _a12 = -a12 / det,
            _a21 = -a21 / det;
        inverse.a11 = _a11;
        inverse.a12 = _a12;
        inverse.a21 = _a21;
        inverse.a22 = _a22;
        float
            _x = -(inverse.a11 * x + inverse.a12 * y);
        inverse.y = -(inverse.a21 * x + inverse.a22 * y);
        inverse.x = _x;
        return true;
    }


    /**
     * Computes bounding box of the quad that spans the axes.
     * @return rectangle describing the bounding box
     */
    public Rectangle getAxesBoundingBox() {
        Rectangle rectangle = new Rectangle(x, y, 0);
        rectangle.expandTo(a11 + x, a21 + y);
        rectangle.expandTo(a12 + x, a22 + y);
        rectangle.expandTo(a11 + a12 + x, a21 + a22 + y);
        return rectangle;
    }

    /**
     * Resolves an orthogonal mapping that transforms segment (X1,Y1),(X2,Y2) into another segment.
     * @param X1          first input point horizontal coordinate
     * @param Y1          first input point vertical coordinate
     * @param X2          second input point horizontal coordinate
     * @param Y2          second input point vertical coordinate
     * @param mappedX1    first mapped point horizontal coordinate
     * @param mappedY1    first mapped point vertical coordinate
     * @param mappedX2    second mapped point horizontal coordinate
     * @param mappedY2    second mapped point vertical coordinate
     * @param minScale    lower limit on mapping scale
     * @param maxScale    upper limit on mapping scale
     * @return the mapping itself after resolving
     */
    public AffineMapping resolve(
        float X1, float Y1, float X2, float Y2,
        float mappedX1, float mappedY1, float mappedX2, float mappedY2,
        float minScale, float maxScale
    ) {
        float
            x1 = X2 - X1,
            y1 = Y2 - Y1,
            n = x1*x1 + y1*y1,
            x2 = (mappedX2 - mappedX1) / n,
            y2 = (mappedY2 - mappedY1) / n;
        a11 = a22 = x1 * x2 + y1 * y2;
        a12 = y1 * x2 - x1 * y2;
        a21 = -a12;
        // respect scale limits
        float scale2 = a11*a22 - a12*a21;
        if (scale2 < minScale * minScale) {
            float f = minScale / (float) Math.sqrt(scale2);
            a11 *= f;
            a12 *= f;
            a21 *= f;
            a22 *= f;
        } else
        if (scale2 > maxScale * maxScale) {
            float f = maxScale / (float) Math.sqrt(scale2);
            a11 *= f;
            a12 *= f;
            a21 *= f;
            a22 *= f;
        }
        float cX = (X1+X2)/2, cY = (Y1+Y2)/2;
        x = (mappedX1+mappedX2)/2 - (a11 * cX + a12 * cY);
        y = (mappedY1+mappedY2)/2 - (a21 * cX + a22 * cY);
        return this;
    }
}