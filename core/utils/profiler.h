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
#include <cstdint>
#include <string>
#include <chrono>
#include <map>
#include <vector>
#include <ostream>

namespace Beatmup {
    /**
        Collects running time statistics of multiple tracks
    */
    class Profiler {
    private:
        typedef uint64_t time_t;
        class Track {
        public:
            time_t min, max, sum;
            uint32_t n;
            Track() : n(0) {}
        };
        
        std::map<std::string, Track> tracks;
        std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> running;
        time_t total;

    public:
        enum class ReportType {
            BRIEF,
            FULL
        };
        Profiler();
        void reset();
        
        void operator ()(const std::string& track);
        void lap();
        void report(std::ostream&, ReportType type = ReportType::FULL) const;

        inline time_t getTotal() const { return total; }
    };

    /**
        Computes moving average over a given number of samples
    */
    template<const size_t log_length, typename datatype=float>
    class MovingAverage {
    private:
        static const size_t MASK = (1 << log_length) - 1;
        datatype samples[1 << log_length], sum;
        size_t idx, level;
    public:
        inline MovingAverage(): sum(0), idx(0), level(0) {}

        inline datatype update(datatype newSample) {
            sum += newSample;
            if (level <= MASK)
                ++level;
            else
                sum -= samples[idx & MASK];
            samples[idx & MASK] = newSample;
            ++idx;
            return sum / level;
        }

        inline datatype operator()() const {
            return level > 0 ? sum / level : 0;
        }
    };
}
