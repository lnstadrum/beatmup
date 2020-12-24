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

#include "matrix.h"
#include "color_spaces.h"
#include <cstring>

using namespace Beatmup;
using namespace Color;

#define MUL(a,b,i,j) (a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j])

Matrix::Matrix() {
    r() = color4f{ 1, 0, 0, 0 };
    g() = color4f{ 0, 1, 0, 0 };
    b() = color4f{ 0, 0, 1, 0 };
    a() = color4f{ 0, 0, 0, 1 };
}

Matrix::Matrix(const color4f& r, const color4f& g, const color4f& b, const color4f& a) {
    this->r() = r;
    this->g() = g;
    this->b() = b;
    this->a() = a;
}

color4f& Matrix::operator[](int row) {
    return rows[row];
}


color4f Matrix::operator[](int row) const {
    return rows[row];
}


Matrix Matrix::operator *(const Matrix& another) const {
    Matrix _;
    for (int i = 0; i < 4; i++) {
        _.elem[i][0] = MUL(elem, another.elem, i, 0);
        _.elem[i][1] = MUL(elem, another.elem, i, 1);
        _.elem[i][2] = MUL(elem, another.elem, i, 2);
        _.elem[i][3] = MUL(elem, another.elem, i, 3);
    }
    return _;
}


void Matrix::operator=(const Matrix& source) {
    memcpy(elem, source.elem, sizeof(elem));
}


Matrix::Matrix(float hDegrees, float saturationFactor, float valueFactor) {
    /*
        R = [1 0 0; 0 cos(H) -sin(H); 0 sin(H) cos(H)];
        A = [1 1 1; 1 / sqrt(2) - 1 / sqrt(2) 0; 1 / sqrt(6) 1 / sqrt(6) - 2 / sqrt(6)]';
        M = simplify(A *[V 0 0; 0 V*S 0; 0 0 V*S] * R * inv(A))
    */
    const float H = hDegrees * (float)pi / 180,
        S = saturationFactor, V = valueFactor;
    r() = { (V*(12 * S*cos(H) + 6)) / 18, -(V*(6 * S*cos(H) + 6 * sqrtf(3)*S*sin(H) - 6)) / 18, (V*(6 * sqrtf(3)*S*sin(H) - 6 * S*cos(H) + 6)) / 18, 0 };
    g() = { (V*(6 * sqrtf(3)*S*sin(H) - 6 * S*cos(H) + 6)) / 18, (V*(12 * S*cos(H) + 6)) / 18, -(V*(S*cos(H) + sqrtf(3)*S*sin(H) - 1)) / 3, 0 };
    b() = {-(V*(6 * S*cos(H) + 6 * sqrtf(3)*S*sin(H) - 6)) / 18, (V*(6 * sqrtf(3)*S*sin(H) - 6 * S*cos(H) + 6)) / 18, (V*(4 * S*cos(H) + 2)) / 6, 0 };
    a() = color4f{ 0, 0, 0, 1 };
}


Matrix::Matrix(const color3f& preservedColor, float saturationFactor, float valueFactor) {
    /*
        R = [1 0 0; 0 cos(H) -sin(H); 0 sin(H) cos(H)];
        A = [1 1 1; 1 / sqrt(2) - 1 / sqrt(2) 0; 1 / sqrt(6) 1 / sqrt(6) - 2 / sqrt(6)]';
        M = simplify(A * inv(R) * [V 0 0; 0 V*S 0; 0 0 -V*S] * R * inv(A))
    */
    const Color::hsva_t hsva(preservedColor);
    const float _2H = 2 * (-hsva.h * 2 * pi - pi / 6),
        S = saturationFactor, V = valueFactor;
    r() = { (V*(S*cos(_2H) - sqrtf(3)*S*sin(_2H) + 1)) / 3, -(V*(2 * S*cos(_2H) - 1)) / 3, (V*(S*cos(_2H) + sqrtf(3)*S*sin(_2H) + 1)) / 3, 0 };
    g() = {-(V*(2 * S*cos(_2H) - 1)) / 3, (V*(S*cos(_2H) + sqrtf(3)*S*sin(_2H) + 1)) / 3, (V*(S*cos(_2H) - sqrtf(3)*S*sin(_2H) + 1)) / 3, 0 };
    b() = { (V*(S*cos(_2H) + sqrtf(3)*S*sin(_2H) + 1)) / 3, (V*(S*cos(_2H) - sqrtf(3)*S*sin(_2H) + 1)) / 3, -(V*(4 * S*cos(_2H) - 2)) / 6, 0 };
    a() = color4f{ 0, 0, 0, 1 };
}
