#include "matrix.h"
#include "color_spaces.h"
#include <cstring>

using namespace Beatmup;
using namespace Color;

#define MUL(a,b,i,j) (a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + a[i][3] * b[3][j])

Matrix::Matrix() {
	r = pixfloat4{ 1, 0, 0, 0 };
	g = pixfloat4{ 0, 1, 0, 0 };
	b = pixfloat4{ 0, 0, 1, 0 };
	a = pixfloat4{ 0, 0, 0, 1 };
}


pixfloat4& Matrix::operator[](int row) {
	return rows[row];
}


pixfloat4 Matrix::operator[](int row) const {
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


Matrix Matrix::getHSVCorrection(float Hdegrees, float S, float V) {
	/*
		R = [1 0 0; 0 cos(H) -sin(H); 0 sin(H) cos(H)];
		A = [1 1 1; 1 / sqrt(2) - 1 / sqrt(2) 0; 1 / sqrt(6) 1 / sqrt(6) - 2 / sqrt(6)]';
		M = simplify(A *[V 0 0; 0 V*S 0; 0 0 V*S] * R * inv(A))
	*/
	const float H = Hdegrees * (float)pi / 180;
	Matrix result;
	result.r = pixfloat4((V*(12 * S*cos(H) + 6)) / 18, -(V*(6 * S*cos(H) + 6 * sqrtf(3)*S*sin(H) - 6)) / 18, (V*(6 * sqrtf(3)*S*sin(H) - 6 * S*cos(H) + 6)) / 18, 0);
	result.g = pixfloat4((V*(6 * sqrtf(3)*S*sin(H) - 6 * S*cos(H) + 6)) / 18, (V*(12 * S*cos(H) + 6)) / 18, -(V*(S*cos(H) + sqrtf(3)*S*sin(H) - 1)) / 3, 0);
	result.b = pixfloat4(-(V*(6 * S*cos(H) + 6 * sqrtf(3)*S*sin(H) - 6)) / 18, (V*(6 * sqrtf(3)*S*sin(H) - 6 * S*cos(H) + 6)) / 18, (V*(4 * S*cos(H) + 2)) / 6, 0);
	return result;
}


Matrix Matrix::getColorInversion(pixfloat3 preservedColor, float S, float V) {
	/*
		R = [1 0 0; 0 cos(H) -sin(H); 0 sin(H) cos(H)];
		A = [1 1 1; 1 / sqrt(2) - 1 / sqrt(2) 0; 1 / sqrt(6) 1 / sqrt(6) - 2 / sqrt(6)]';
		M = simplify(A * inv(R) * [V 0 0; 0 V*S 0; 0 0 -V*S] * R * inv(A))
	*/
	pixhsva hsva(preservedColor);
	const float _2H = 2 * (-hsva.h * 2 * pi - pi / 6);
	Matrix result;
	result.r = pixfloat4((V*(S*cos(_2H) - sqrtf(3)*S*sin(_2H) + 1)) / 3, -(V*(2 * S*cos(_2H) - 1)) / 3, (V*(S*cos(_2H) + sqrtf(3)*S*sin(_2H) + 1)) / 3, 0);
	result.g = pixfloat4(-(V*(2 * S*cos(_2H) - 1)) / 3, (V*(S*cos(_2H) + sqrtf(3)*S*sin(_2H) + 1)) / 3, (V*(S*cos(_2H) - sqrtf(3)*S*sin(_2H) + 1)) / 3, 0);
	result.b = pixfloat4((V*(S*cos(_2H) + sqrtf(3)*S*sin(_2H) + 1)) / 3, (V*(S*cos(_2H) - sqrtf(3)*S*sin(_2H) + 1)) / 3, -(V*(4 * S*cos(_2H) - 2)) / 6, 0);
	return result;
}
