/**
	Rectangular area
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

		inline void translate(numeric X, numeric Y) {
			x += X;
			y += Y;
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
		CustomPoint<numeric> A, B;

		CustomRectangle<numeric>() : A(0, 0), B(0, 0)
		{}

		CustomRectangle<numeric>(CustomPoint<numeric> A, CustomPoint<numeric> B) : A(A), B(B)
		{}

		CustomRectangle<numeric>(numeric x1, numeric y1, numeric x2, numeric y2) : A(CustomPoint<numeric>(x1, y1)), B(CustomPoint<numeric>(x2, y2))
		{}

		inline CustomRectangle<numeric> operator*(numeric X) const {
			return CustomRectangle<numeric>(A * X, B * X);
		}

		inline CustomRectangle<numeric> operator/(numeric X) const {
			return CustomRectangle<numeric>(A / X, B / X);
		}


		numeric width() const {
			return B.x - A.x;
		}

		numeric height() const {
			return B.y - A.y;
		}
		
		/**
			Computes the rectangle area
		*/
		numeric getArea() const {
			return (B.x - A.x) * (B.y - A.y);
		}

		/**
			Filps corners coordinates guaranteeing that it has a non negative area, i.e. A <= B (componentwise)
		*/
		void normalize() {
			order<numeric>(A.x, B.x);
			order<numeric>(A.y, B.y);
		}

		/**
			Translates the box
		*/
		inline void translate(numeric X, numeric Y) {
			A.translate(X, Y);
			B.translate(X, Y);
		}

		/**
			Scales the box
		*/
		inline void scale(numeric X, numeric Y) {
			A.x *= X; A.y *= Y;
			B.x *= X; B.y *= Y;
		}

		/**
			Truncates a rectangle to a limiting frame
		*/
		inline void limit(const CustomRectangle<numeric>& frame) {
			if (A.x < frame.A.x)
				A.x = frame.A.x;
			if (A.y < frame.A.y)
				A.y = frame.A.y;
			if (B.x > frame.B.x)
				B.x = frame.B.x;
			if (B.y > frame.B.y)
				B.y = frame.B.y;
		}

		/**
			Returns a translated box
		*/
		inline CustomRectangle<numeric> translated(numeric X, numeric Y) {
			return CustomRectangle<numeric>(
				CustomPoint<numeric>(A.x + X, A.y + Y),
				CustomPoint<numeric>(B.x + X, B.y + Y)
			);
		}

		/**
			Test if a point is inside the rectangle (or on its the border)
		*/
		bool isInside(const CustomPoint<numeric>& P) const {
			return (A.x <= P.x) && (P.x <= B.x) && (A.y <= P.y) && (P.y <= B.y);
		}

		/**
			Test if a point is inside the rectangle including left and top borders, but excluding right and bottom
		*/
		bool isInsideHalfOpened(const CustomPoint<numeric>& P) const {
			return (A.x <= P.x) && (P.x < B.x) && (A.y <= P.y) && (P.y < B.y);
		}

		/**
			Rectangle positionning test with respect to a given vertical line
			\returns -1 if the line passes on the left side of the rectangle, 1 if it is on the right side, 0 otherwise
		*/
		short int horizontalPositioningTest(numeric X) const {
			if (X < A.x)
				return -1;
			if (X > B.x)
				return 1;
			return 0;
		}

		/**
			Rectangle positionning test with respect to a given horizontal line
			\returns -1 if the line passes above the rectangle, 1 if it passes below, 0 otherwise
		*/
		short int verticalPositioningTest(numeric Y) const {
			if (Y < A.y)
				return -1;
			if (Y > B.y)
				return 1;
			return 0;
		}

		void grow(numeric r) {
			A.x -= r;
			A.y -= r;
			B.x += r;
			B.y += r;
		}

		/**
			Typecast to float-valued coordinates
		*/
		inline operator CustomRectangle<float>() const {
			CustomRectangle<float> r((CustomPoint<float>)A, (CustomPoint<float>)B);
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

		/**
			Computes the corresponding transformed point of the point (x,y)
		*/
		inline CustomPoint<numeric> operator()(numeric X, numeric Y) const {
			return CustomPoint<numeric>(a11 * X + a12 * Y, a21 * X + a22 * Y);
		}

		/**
			Integer overloading of (x,y) operator to avoid warnings
		*/
		inline CustomPoint<numeric> operator()(int X, int Y) const {
			return this->operator()((float)X, (float)Y);
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
		inline CustomMatrix2<numeric> operator*(const CustomMatrix2<numeric>& matrix) const {
			CustomMatrix2<numeric> result;
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
		inline CustomMatrix2<numeric> operator*(const numeric factor) const {
			CustomMatrix2<numeric> result;
			result.a11 = a11*factor;
			result.a12 = a12*factor;
			result.a21 = a21*factor;
			result.a22 = a22*factor;
			return result;
		}

		void scale(numeric factor) {
			scale(factor, factor);
		}

		void scale(numeric X, numeric Y) {
			a11 *= X;
			a12 *= Y;
			a21 *= X;
			a22 *= Y;
		}

		void prescale(numeric X, numeric Y) {
			a11 *= X;
			a12 *= X;
			a21 *= Y;
			a22 *= Y;
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
		CustomPoint<numeric> getInverse(const CustomPoint<numeric>& p) const {
			numeric D = det();
			return CustomPoint<numeric>(
				(+p.x * a22 - p.y * a12) / D,
				(-p.x * a21 + p.y * a11) / D
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
			Changes transformation domain and codomain units
		*/
		void rescaleUnits(numeric Xdom, numeric Ydom, numeric Xcod, numeric Ycod) {
			a11 = a11 * Xcod / Xdom;
			a12 = a12 * Xcod / Ydom;
			a21 = a21 * Ycod / Xdom;
			a22 = a22 * Ycod / Ydom;
		}

		/**
			Checks whether a given point is inside a rectangular area in the transform codomain
		*/
		template<typename num> inline bool isPointInsideBox(num x, num y, CustomRectangle<num> box) const {
			num p = a11*x + a12*y;
			if (p < box.A.x || box.B.x < p)
				return false;
			p = a21*x + a22*y;
			return (box.A.y <= p  &&  p <= box.B.y);
		}

		/**
			Checks whether a given point is inside the unit square in the affine axes
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
		Entity representing 2x3 affine mapping regrouping Matrix2 and Point
	*/
	class AffineMapping {
	public:
		Matrix2 matrix;
		Point position;

		AffineMapping();
		AffineMapping(const Matrix2& aMatrix, const Point& aPosition);


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
			Computes and return inversed mapping
		*/
		AffineMapping getInverse() const;

		/**
			Computes inversed point
		*/
		Point getInverse(const Point& pos) const;
		Point getInverse(float x, float y) const;

		/**
			Set center position of the axes box
		*/
		void setCenterPosition(const Point& newPos);

		/**
			Translates a mapping
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
			Tests whether a point in TARGET domain is inside axes span
		*/
		bool isPointInside(const Point& point) const;
		bool isPointInside(float x, float y) const;
		bool isPointInside(float x, float y, float width, float height) const;
	};
}