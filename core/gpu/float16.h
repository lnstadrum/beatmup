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
#include <string>
#include <cstdint>
#include <cmath>

namespace Beatmup {
    namespace GL {
        /**
            16 bit floating point representation.
            The exponent and the signed fractional part are encoded to 8 bits each.
                x = frac * 2 ^ exp
        */
        class Float16 {
        private:
            uint8_t frac;   //!< fractional part
            uint8_t exp;    //!< exponent part

        public:
            /**
                Encodes a given floating point value in 16 bit.
                \param input        The input floating point value
            */
            inline Float16(float input) {
                const int exponent = (int)(std::log2(std::abs(input)) + 1.0f);
                const float frac = std::ldexp(input, -exponent) * 128.0f + 128.5f;
                this->frac = frac < 0.0f ? 0 : frac > 255.0f ? 255 : (uint8_t)frac;
                this->exp = (uint8_t)(exponent + 128);
            }

            /**
                Constructs a Float16 number from the fractional part and the exponent values given explicitly.
                \param frac         The fractional part
                \param exponent     The exponent
            */
            inline Float16(uint8_t frac, uint8_t exp): frac(frac), exp(exp) {}

            /**
                Converts the encoded value to a floating-point value.
            */
            inline operator float() const {
                return std::ldexp(frac / 128.0f - 1.0f, (int)exp - 128);
            }

            /**
                Returns encoded exponent.
            */
            inline uint8_t getExp() const {
                return exp;
            }

            /**
                Returns encoded fractional part.
            */
            inline uint8_t getFrac() const {
                return frac;
            }

            /**
                Returns a GLSL code defining the encoding function from a high precision floating point value into a low precision vec2.
                \param name     The function name
            */
            static inline std::string encodeGlsl(const std::string& name = "encode") {
                return "lowp vec2 " + name + R"((highp float value) {
                    highp float e  = floor(log2(abs(value)) + 1.0);
                    return vec2(value * exp2(-e - 1.0) + (0.5 + 0.5/255.0), (e + 128.0) / 255.0);
                })";
            }

            /**
                Returns a GLSL code defining the decoding function from a low precision vec2 to a high precision floating point value.
                \param name     The function name
            */
            static inline std::string decodeGlsl(const std::string& name = "decode") {
                return "highp float " + name + R"((lowp vec2 pack) {
                    highp float e = floor(pack.y * 255.0 - 127.5);
                    return (floor(pack.x * 255.0) / 128.0 - 1.0) * exp2(e);
                })";
            }
        };
    }
}