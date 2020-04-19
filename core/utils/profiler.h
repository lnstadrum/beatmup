/*
    Execution time profiling tool
*/

#pragma once
#include <stdint.h>
#include <string>
#include <chrono>
#include <map>
#include <vector>
#include <ostream>

namespace Beatmup {
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
    };
}