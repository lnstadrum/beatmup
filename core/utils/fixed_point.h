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
#include "../basic_types.h"
#include <cstdint>
#include <limits>

namespace Beatmup {

    /**
        Fixed-point number.
        \tparam rtype       Datatype used to store the values
        \tparam precision   Number of bits to represent the fractional part
        \tparam optype      Datatype used in arithmetic operations
    */
    template<typename rtype, const int precision, typename optype>
    struct Fixed {
    protected:
        rtype value;
        inline static rtype clamp(const float val, const rtype min, const rtype max) {
            return val <= min ? min : val >= max ? max : (rtype)roundf_fast(val);
        }

        static inline rtype fromFloat(const float val) {
            return clamp(val * (1 << precision), std::numeric_limits<underlying_type>::min(), std::numeric_limits<underlying_type>::max());
        }

        Fixed(const rtype value) : value(((optype)value) << precision) {}

    public:
        typedef rtype underlying_type;

        Fixed() : value(0) {}

        Fixed(const Fixed& another): value(another.value) {}

        Fixed(float value) : value(fromFloat(value)) {}

        Fixed& operator=(const float value) {
            this->value = fromFloat(value);
            return *this;
        }

        Fixed& operator=(const rtype value) {
            this->value = ((optype)value) << precision;
            return *this;
        }

        Fixed operator+(const Fixed& another) const {
            return Fixed::interpret( static_cast<rtype>(value + another.value) );
        }

        Fixed operator-(const Fixed& another) const {
            return Fixed::interpret( static_cast<rtype>(value - another.value) );
        }

        Fixed operator*(const Fixed& another) const {
            return Fixed::interpret( static_cast<rtype>(((optype)value * another.value) >> precision) );
        }

        Fixed operator/(const Fixed& another) const {
            return Fixed::interpret( static_cast<rtype>(((optype)value << precision) / another.value) );
        }


        operator float() const {
            return (float)value / (float)(1 << precision);
        }


        uint8_t asTexture() const {
            const optype v = (((optype)value) * 255) >> precision;
            return v <= 0 ? 0 : v >= 255 ? 255 : (uint8_t)v;
        }


        static Fixed interpret(const underlying_type value) {
            Fixed result;
            result.value = value;
            return result;
        }


        /**
            \return min representable value.
        */
        static float min() {
            return interpret(std::numeric_limits<underlying_type>::min());
        }


        /**
            \return max representable value.
        */
        static float max() {
            return interpret(std::numeric_limits<underlying_type>::max());
        }
    };


    /**
        Signed 16-bit fixed point
        With default precision represents [-1, 1) range.
    */
    template<const int precision = 15> using Fixed16 = Fixed<int16_t, precision, int32_t>;
}
