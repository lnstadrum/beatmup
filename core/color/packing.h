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

namespace Beatmup {
    color4i fromPackedInt(int32_t _) {
        return color4i{ (uint8_t)(_ & 0xff), (uint8_t)((_ >> 8) & 0xff), (uint8_t)(_ >> 16), (uint8_t)((_ >> 24) & 0xff) };
    }
}
