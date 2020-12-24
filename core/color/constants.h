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

namespace Beatmup{
    namespace Color {
        static const color4i
            WHITE               = { 255, 255, 255, 255 },
            BLACK               = {   0,   0,   0, 255 },
            DARK_SEA_GREEN1       = { 193, 255, 193, 255 },
            DARK_SEA_GREEN2       = { 180, 238, 180, 255 },
            FECAMP_SKY           = { 100, 140, 189, 255 },
            TRANSPARENT_BLACK    = {   0,   0,   0,   0 },
            ZERO = TRANSPARENT_BLACK;


        static const color4f
            TRANSPARENT_BLACK_F    = {   0,   0,   0,   0 },
            ZERO_F = TRANSPARENT_BLACK_F;
    }
}