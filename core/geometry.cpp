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

#include "geometry.h"
#include <limits>

using namespace Beatmup;

const AffineMapping AffineMapping::IDENTITY(Matrix2::IDENTITY, Point::ZERO);


AffineMapping::AffineMapping() {}

AffineMapping::AffineMapping(const Matrix2& aMatrix, const Point& aPosition) :
    matrix(aMatrix), position(aPosition)
{}

AffineMapping::AffineMapping(const Rectangle& rectangle):
    matrix(rectangle.width(), rectangle.height()), position(rectangle.getX1(), rectangle.getY1())
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

void AffineMapping::translate(const Point& shift) {
    position = position + shift;
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
