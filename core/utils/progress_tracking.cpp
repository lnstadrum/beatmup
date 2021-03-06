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

#include "progress_tracking.h"
#include <algorithm>
#include <cstdio>


Beatmup::ProgressTracking Beatmup::ProgressTracking::DEVNULL;


void Beatmup::ProgressTracking::printOutProgressBar(const unsigned int barLength) const {
    printf("\r[");
    const unsigned int val = std::min(barLength - 2, progress * (barLength - 2) / maxProgress);
    for (unsigned int i = 0; i < val; ++i)
        printf("*");
    for (unsigned int i = val; i < barLength - 2; ++i)
        printf(" ");
    printf("]");
    fflush(stdout);
}
