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

#pragma once

namespace Beatmup {
    /**
        Progress tracking utility.
    */
    class ProgressTracking {
    private:
        unsigned int progress, maxProgress;
    public:
        static ProgressTracking DEVNULL;

        inline ProgressTracking() : progress(0), maxProgress(1) {}
        inline ProgressTracking(unsigned int max) : progress(0), maxProgress(max) {}

        /**
            Resets the progress to zero.
            \param[in] max      New maximum progress
        */
        inline void reset(unsigned int max) {
            this->progress = 0;
            this->maxProgress = max;
        }

        /**
            Increases progress by one step.
        */
        inline void operator()() { progress++; }

        /**
            \return `true` when the tracked activity is over.
        */
        inline bool done() const { return progress >= maxProgress; }

        /**
            Prints a fancy progress bar to standard output.
            Designed to be called repeatedly: prints on the same line erasing the previously printed bar.
            \param[in] barLength        The length of the progress bar (number of characters)
        */
        void printOutProgressBar(const unsigned int barLength = 40) const;
    };

}
