/**
    Geometry datatypes and routines
*/

#pragma once
#include "basic_types.h"
#include <cmath>

#define between(A,X,B) (((A) <= (X) && (X) <= (B)) || ((A) >= (X) && (X) >= (B)))
#define sqr(X) ((X)*(X))

namespace Beatmup {

	template<typename T> inline void order(T& a, T& b) {
		if (a > b) {
			T _;
			_ = a; a = b; b = _;
        }
	}
	

	/**
		2D point class
	*/
	template<typename numeric> class CustomPoint {
	public:
		numeric x, y;

        inline numeric getX() const {
            return x;
        }

        inline numeric getY() const {
            return y;
        }

		inline bool operator==(const CustomPoint<numeric>& _) const {
			return x == _.x && y == _.y;
		}

		inline CustomPoint<numeric> operator+(const CustomPoint<numeric>& _) const {
			return CustomPoint<numeric>(x + _.x, y + _.y);
		}

		inline CustomPoint<numeric> operator-(const CustomPoint<numeric>& _) const {
			return CustomPoint<numeric>(x - _.x, y - _.y);
		}

		inline CustomPoint<numeric> operator+(numeric _) const {
			return CustomPoint<numeric>(x + _, y + _);
		}

		inline CustomPoint<numeric> operator-(numeric _) const {
			return CustomPoint<numeric>(x - _, y - _);
		}

		inline CustomPoint<numeric> operator*(numeric _) const {
			return CustomPoint<numeric>(x * _, y * _);
		}

		inline CustomPoint<numeric> operator/(numeric _) const {
			return CustomPoint<numeric>(x / _, y / _);
		}

		inline void translate(numeric x, numeric y) {
			this->x += x;
			this->y += y;
		}

		inline CustomPoint<numeric>() { x = y = 0; }

		inline CustomPoint<numeric>(numeric x, numeric y) : x(x), y(y) {}

		inline numeric hypot2() const {
			return x*x + y*y;
		}

		inline bool isInsideAxesSpan(numeric scaleX, numeric scaleY) const {
			return 0 <= x && x <= scaleX && 0 <= y && y <= scaleY;
		}

		/**
			Typecast to float-valued coordinates
		*/
		inline operator CustomPoint<float>() const {
			CustomPoint<float> p((float)x, (float)y);
			return p;
		}

		/**
			Typecast to integer valued coordinates
		*/
		inline operator CustomPoint<int>() const {
			CustomPoint<int> p((int)(x), (int)(y));
			return p;
		}

		static const CustomPoint < numeric >
			ZERO;			// zero point of type numeric
	};

	
	/**
		2D rectangle class
		All the utilities assume that the rectangle is normalized, e.g. its area is not negative
	*/
	template<typename numeric> class CustomRectangle {
	public:
		CustomPoint<numeric> a, b;

		CustomRectangle() : a(0, 0), b(0, 0)
		{}

		CustomRectangle(CustomPoint<numeric> a, CustomPoint<numeric> b) : a(a), b(b)
		{}

		CustomRectangle(numeric x1, numeric y1, numeric x2, numeric y2) : a(CustomPoint<numeric>(x1, y1)), b(CustomPoint<numeric>(x2, y2))
		{}

		inline CustomRectangle operator*(numeric _) const {
			return CustomRectangle(_ * a, _ * b);
		}

		inline CustomRectangle operator/(numeric _) const {
			return CustomRectangle(a / _, b / _);
		}

        inline numeric getX1() const { return a.x; }
        inline numeric getY1() const { return a.y; }
        inline numeric getX2() const { return b.x; }
        inline numeric getY2() const { return b.y; }

		numeric width() const {
			return b.x - a.x;
		}

		numeric height() const {
			return b.y - a.y;
		}
		
		/**
			Computes the rectangle area
		*/
		numeric getArea() const {
			return (b.x - a.x) * (b.y - a.y);
		}

		/**
			Filps corners coordinates guaranteeing that it has a non negative area, i.e. a <= b (componentwise)
		*/
		void normalize() {
			order<numeric>(a.x, b.x);
			order<numeric>(a.y, b.y);
		}

		/**
			Translates the box
		*/
		inline void translate(numeric x, numeric y) {
			a.translate(x, y);
			b.translate(x, y);
		}

		/**
			Scales the box
		*/
		inline void scale(numeric x, numeric y) {
			a.x *= x; a.y *= y;
			b.x *= x; b.y *= y;
		}

		/**
			Truncates a rectangle to a limiting frame
		*/
		inline void limit(const CustomRectangle& frame) {
			if (a.x < frame.a.x)
				a.x = frame.a.x;
			if (a.y < frame.a.y)
				a.y = frame.a.y;
			if (b.x > frame.b.x)
				b.x = frame.b.x;
			if (b.y > frame.b.y)
				b.y = frame.b.y;
		}

		/**
			Returns a translated box
		*/
		inline CustomRectangle translated(numeric x, numeric y) {
			return CustomRectangle(
				CustomPoint<numeric>(a.x + x, a.y + y),
				CustomPoint<numeric>(b.x + x, b.y + y)
			);
		}

		/**
			Test if a point is inside the rectangle (or on its the border)
		*/
		bool isInside(const CustomPoint<numeric>& point) const {
			return (a.x <= point.x) && (point.x <= b.x) && (a.y <= point.y) && (point.y <= b.y);
		}

		/**
			Test if a point is inside the rectangle including left and top borders, but excluding right and bottom
		*/
		bool isInsideHalfOpened(const CustomPoint<numeric>& point) const {
			return (a.x <= point.x) && (point.x < b.x) && (a.y <= point.y) && (point.y < b.y);
		}

		/**
			Rectangle positionning test with respect to a given vertical line
			\returns -1 if the line passes on the left side of the rectangle, 1 if it is on the right side, 0 otherwise
		*/
		short int horizontalPositioningTest(numeric x) const {
			if (x < a.x)
				return -1;
			if (x > b.x)
				return 1;
			return 0;
		}

		/**
			Rectangle positionning test with respect to a given horizontal line
			\returns -1 if the line passes above the rectangle, 1 if it passes below, 0 otherwise
		*/
		short int verticalPositioningTest(numeric y) const {
			if (y < a.y)
				return -1;
			if (y > b.y)
				return 1;
			return 0;
		}

		void grow(numeric r) {
			a.x -= r;
			a.y -= r;
			b.x += r;
			b.y += r;
		}

		/**
			Typecast to float-valued coordinates
		*/
		inline operator CustomRectangle<float>() const {
			CustomRectangle<float> r((CustomPoint<float>)a, (CustomPoint<float>)b);
			return r;
		}
	};
	

	/**
		2D affine transformation
		Defines operators (...) to transform the points and a set of useful utilities to work with affine mappings
	*/
	template<typename numeric> class CustomMatrix2 {
	private:
		numeric a11, a12, a21, a22;		//!< transformation matrix coefficients
	public:
		CustomMatrix2() : a11(1), a12(0), a21(0), a22(1)
		{}

		CustomMatrix2(numeric lambda1, numeric lambda2) : a11(lambda1), a12(0), a21(0), a22(lambda2)
		{}

		CustomMatrix2(numeric _11, numeric _12, numeric _21, numeric _22): a11(_11), a12(_12), a21(_21), a22(_22)
        {}

		inline numeric getA11() const { return a11; }
		inline numeric getA12() const { return a12; }
		inline numeric getA21() const { return a21; }
		inline numeric getA22() const { return a22; }

		/**
			Computes the corresponding transformed point of the point (x,y)
		*/
		inline CustomPoint<numeric> operator()(numeric x, numeric y) const {
			return CustomPoint<numeric>(a11 * x + a12 * y, a21 * x + a22 * y);
		}

		/**
			Integer overloading of (x,y) operator to avoid warnings
		*/
		inline CustomPoint<numeric> operator()(int x, int y) const {
			return this->operator()((float)x, (float)y);
		}

		/**
			Computes transformed point of a given one
		*/
		inline CustomPoint<numeric> operator()(const CustomPoint<numeric>& point) const {
			return CustomPoint<numeric>(a11 * point.x + a12 * point.y, a21 * point.x + a22 * point.y);
		}

		/**
			Multiplies two matrices
		*/
		inline CustomMatrix2 operator*(const CustomMatrix2& matrix) const {
			CustomMatrix2 result;
			result.a11 = a11 * matrix.a11 + a12 * matrix.a21;
			result.a12 = a11 * matrix.a12 + a12 * matrix.a22;
			result.a21 = a21 * matrix.a11 + a22 * matrix.a21;
			result.a22 = a21 * matrix.a12 + a22 * matrix.a22;
			return result;
		}

		/**
			Multiplies matrix by a vector
		*/
		inline CustomPoint<numeric> operator*(const CustomPoint<numeric>& point) const {
			return this->operator()(point);
		}


		/**
			Multiplies matrix by a scalar
		*/
		inline CustomMatrix2 operator*(const numeric factor) const {
			CustomMatrix2 result;
			result.a11 = a11*factor;
			result.a12 = a12*factor;
			result.a21 = a21*factor;
			result.a22 = a22*factor;
			return result;
		}

		void scale(numeric factor) {
			scale(factor, factor);
		}

		void scale(numeric x, numeric y) {
			a11 *= x;
			a12 *= y;
			a21 *= x;
			a22 *= y;
		}

		void prescale(numeric x, numeric y) {
			a11 *= x;
			a12 *= x;
			a21 *= y;
			a22 *= y;
		}

		void rotateRadians(float angle) {
			float
				c = cos(angle),
				s = sin(angle),
				_11 = c * a11 + s * a12,
				_21 = c * a21 + s * a22;
			a12 = -s * a11 + c * a12;
			a22 = -s * a21 + c * a22;
			a11 = _11;
			a21 = _21;
		}
		
		inline void rotateDegrees(float angle) {
			rotateRadians(angle * pi / 180.0f);
		}

		inline void skewRadians(float x, float y) {
			float
				tx = tan(x), ty = tan(y),
				_11 = a11 * (1 + tx*ty) + a12 * ty,
				_21 = a21 * (1 + tx*ty) + a22 * ty,
				_12 = a11 * tx + a12;
			a22 = a21 * tx + a22;
			a11 = _11;
			a12 = _12;
			a21 = _21;
		}

		inline void skewDegrees(float x, float y) {
			skewRadians(x * pi / 180.0f, y * pi / 180.0f);
		}

		/**
			Transformation determinant
		*/
		numeric det() const {
			return a11*a22 - a12*a21;
		}
		
		bool isInvertible() const {
			return det() != 0;
		}

		/**
			Computes inversed transformation
			\returns the inverse
		*/
		CustomMatrix2 getInverse() const {
			numeric D = det();
			CustomMatrix2 i;
			i.a11 = a22 / D;
			i.a22 = a11 / D;
			i.a12 = -a12 / D;
			i.a21 = -a21 / D;
			return i;
		}

		/**
			Computes inverse of a given point
			\returns the inverse
		*/
		CustomPoint<numeric> getInverse(numeric x, numeric y) const {
			numeric D = det();
			return CustomPoint<numeric>(
				( x * a22 - y * a12) / D,
				(-x * a21 + y * a11) / D
			);			
		}

		/**
			Computes inverse of a given point
			\returns the inverse
		*/
		CustomPoint<numeric> getInverse(const CustomPoint<numeric>& point) const {
			const numeric det = det();
			return CustomPoint<numeric>(
				(+point.x * a22 - point.y * a12) / det,
				(-point.x * a21 + point.y * a11) / det
			);
		}

		/**
			\returns transposed matrix
		*/
		CustomMatrix2 getTransposed() const {
			CustomMatrix2 t;
			t.a11 = a11;
			t.a12 = a21;
			t.a21 = a12;
			t.a22 = a22;
			return t;
		}

		/**
			Scales transformation input and output units
            If input/output axes change their scales, the transformation may be rescaled to keep
            the correspondence between the same points as before but in newly scaled coordinates.
		*/
		void rescaleUnits(numeric xIn, numeric yIn, numeric xOut, numeric yOut) {
			a11 = a11 * xOut / xIn;
			a12 = a12 * xOut / yIn;
			a21 = a21 * yOut / xIn;
			a22 = a22 * yOut / yIn;
		}

		/**
			Checks whether a given input point is inside a rectangular area when transformed
		*/
		template<typename num> inline bool isPointInsideBox(num x, num y, CustomRectangle<num> box) const {
			num p = a11*x + a12*y;
			if (p < box.a.x || box.b.x < p)
				return false;
			p = a21*x + a22*y;
			return (box.a.y <= p  &&  p <= box.b.y);
		}

		/**
			Checks whether a given input point is inside the unit square when transformed
		*/
		inline bool isPointInsideAxes(numeric x, numeric y, numeric w, numeric h) const {
			numeric p = a11*x + a12*y;
			if (p < 0 || w < p)
				return false;
			p = a21*x + a22*y;
			return (0 <= p  &&  p <= h);
		}

		inline bool isPointInsideAxes(numeric x, numeric y) const {
			return isPointInsideAxes(x, y, 1, 1);
		}

		/**
			Computes X axis scaling factor
		*/
		inline numeric getScalingX() {
			return sqrt(sqr(a11) + sqr(a21));
		}

		/**
			Computes Y axis scaling factor
		*/
		inline numeric getScalingY() {
			return sqrt(sqr(a12) + sqr(a22));
		}
		
		/**
			Returns first axis orientation in degrees
		*/
		inline float getOrientationDegrees() {
			return atan2(a11, a12) *180/pi;
		}

		/**
			Retrieves matrix element values
		*/
		inline void getElements(float& a11, float& a12, float& a21, float& a22) const {
			a11 = this->a11;
			a12 = this->a12;
			a21 = this->a21;
			a22 = this->a22;
		}

		/**
			Sets matrix element values
		*/
		inline void setElements(float a11, float a12, float a21, float a22) {
			this->a11 = a11;
			this->a12 = a12;
			this->a21 = a21;
			this->a22 = a22;
		}
      
        static const CustomMatrix2 IDENTITY;
	};

	
	/**
		Point with negative coordinates
	*/
	template<typename numeric> inline CustomPoint<numeric> operator-(const CustomPoint<numeric>& point) {
		return CustomPoint<numeric>(-point.x, -point.y);
	}

	/**
		Product of a scalar and a point
	*/
	template<typename numeric> inline CustomPoint<numeric> operator*(numeric val, const CustomPoint<numeric>& point) {
		return point * val;
	}

	/**
		Product of a scalar and a matrix
	*/
	template<typename numeric> inline CustomMatrix2<numeric> operator*(numeric val, const CustomMatrix2<numeric>& matrix) {
		return matrix * val;
	}

	typedef CustomPoint<float> Point;
	typedef CustomRectangle<float> Rectangle;
	typedef CustomMatrix2<float> Matrix2;

	typedef CustomPoint<int> IntPoint;
	typedef CustomRectangle<int> IntRectangle;

	/**
		2x3 affine mapping regrouping Matrix2 and Point
	*/
	class AffineMapping {
	public:
		Matrix2 matrix;
		Point position;

		AffineMapping();
		AffineMapping(const Matrix2& aMatrix, const Point& aPosition);
      
		inline Point getPosition() const {
			return position;
        }

		inline Matrix2 getMatrix() const {
			return matrix;
        }

		inline Point operator()(const Point& point) const {
			return matrix(point) + position;
		}
		
		/**
			Composition of two mappings
		*/
		AffineMapping operator*(const AffineMapping& mapping) const;

		void setIdentity();

		/**
			Inverts the mapping itself
		*/
		void invert();

		/**
			Computes inversed mapping
		*/
		AffineMapping getInverse() const;

		/**
			Computes inverse of a point
		*/
		Point getInverse(const Point& pos) const;
		Point getInverse(float x, float y) const;

		/**
			Set center position of the axes box
		*/
		void setCenterPosition(const Point& newPos);

		/**
			Translates the mapping
		*/
		void translate(const Point& delta);
		
		/**
			Scales a mapping around specified point in target domain
		*/
		void scale(float factor, const Point& fixedPoint = Point::ZERO);

		/**
			Rotates a mapping around specified point in target domain
		*/
		void rotateDegrees(float angle, const Point& fixedPoint = Point::ZERO);

		/**
			Tests whether a point in output domain is inside the input axes span
		*/
		bool isPointInside(const Point& point) const;
		bool isPointInside(float x, float y) const;
		bool isPointInside(float x, float y, float width, float height) const;
      
        static const AffineMapping IDENTITY;
	};
}