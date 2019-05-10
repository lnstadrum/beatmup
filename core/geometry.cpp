#include "geometry.h"
#include <limits>

using namespace Beatmup;

template<> const Point Point::ZERO = Point(0, 0);
template<> const IntPoint IntPoint::ZERO = IntPoint(0, 0);


AffineMapping::AffineMapping() {}

AffineMapping::AffineMapping(const Matrix2& aMatrix, const Point& aPosition) :
	matrix(aMatrix), position(aPosition)
{}


AffineMapping AffineMapping::operator*(const AffineMapping& mapping) const {	
	return AffineMapping(matrix * mapping.matrix, position + matrix * mapping.position);
}

void AffineMapping::setIdentity() {
	matrix.setElements(1.0f, 0.0f, 0.0f, 1.0f);
	position = Point::ZERO;
}

void AffineMapping::invert() {
	matrix = matrix.getInverse();
	position = -(matrix * position);
}


AffineMapping AffineMapping::getInverse() const {
	AffineMapping inverse;
	inverse.matrix = matrix.getInverse();
	inverse.position = - ( inverse.matrix * position );
	return inverse;
}


Point AffineMapping::getInverse(const Point& point) const {
	return matrix.getInverse(point.x - position.x, point.y - position.y);
}


Point AffineMapping::getInverse(float x, float y) const {
	return matrix.getInverse(x - position.x, y - position.y);
}


void AffineMapping::setCenterPosition(const Point& newPos) {
	position = newPos - matrix(0.5f, 0.5f);
}


void AffineMapping::translate(const Point& delta) {
	position = position + delta;
}


void AffineMapping::scale(float factor, const Point& fixedPoint) {
	const Point pos0 = matrix(fixedPoint);
	matrix.scale(factor);
	position = position + pos0 - matrix * fixedPoint;
}


void AffineMapping::rotateDegrees(float angle, const Point& fixedPoint) {
	const Point pos0 = matrix(fixedPoint);
	matrix.rotateDegrees(angle);
	position = position + pos0 - matrix * fixedPoint;
}


bool AffineMapping::isPointInside(const Point& point) const {
	return isPointInside(point.x, point.y);
}


bool AffineMapping::isPointInside(float x, float y) const {
	return matrix.getInverse().isPointInsideAxes(x - position.x, y - position.y);
}


bool AffineMapping::isPointInside(float x, float y, float width, float height) const {
	return matrix.getInverse().isPointInsideAxes(x - position.x, y - position.y, width, height);
}