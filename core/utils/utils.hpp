/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#pragma once

#define ceili(x,y) (((x)+(y)-1)/(y))				//!< integer division x/y with ceiling
#define roundf_fast(X) (floorf_fast((X) + 0.5f))	//!< rounding (nearest integer)


// fast floorf (no range check)
inline int floorf_fast(float x) {
    int i = (int)x;
    return i - (i > x);
}

// modulus
inline float modf(float x, float y) {
    return x - y * (int)(x / y);
}
