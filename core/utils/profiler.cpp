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

#include "profiler.h"
#include "../exception.h"
#include <algorithm>
#include <iomanip>

using namespace Beatmup;

Profiler::Profiler() : total(0) {}

void Beatmup::Profiler::reset() {
    tracks.clear();
    total = 0;
}

void Profiler::operator()(const std::string& track) {
    running.emplace_back(track, std::chrono::system_clock::now());
}

void Profiler::lap() {
    BEATMUP_ASSERT_DEBUG(!running.empty());
    std::chrono::system_clock::time_point endTime(std::chrono::system_clock::now());
    Track& track = tracks[running.back().first];
    time_t sample = (time_t)std::chrono::duration<float, std::micro>(endTime - running.back().second).count();
    running.pop_back();
    if (track.n == 0) {
        track.min = track.max = track.sum = sample;
        track.n = 1;
    }
    else {
        track.n++;
        track.sum += sample;
        track.min = std::min(track.min, sample);
        track.max = std::max(track.max, sample);
    }
    total += sample;
}

void Profiler::report(std::ostream& stream, ReportType type) const {
    if (tracks.empty())
        return;

    typedef std::pair<std::string, time_t> Entry;
    std::vector<Entry> idx;
    idx.reserve(tracks.size());
    size_t maxlen = 0;
    for (const auto& _ : tracks) {
        idx.emplace_back(_.first, _.second.n == 0 ? 0 : (_.second.sum / _.second.n));
        maxlen = std::max(_.first.size(), maxlen);
    }
    std::sort(idx.begin(), idx.end(), [&](Entry& _1, Entry& _2) { return _1.second < _2.second; });
    stream << "=== " << total << " us" << std::endl;
    if (type == ReportType::FULL) {
        stream
            << std::setw(maxlen) << "<id>"
            << "\t" << "<avg>"
            << "\t" << "<min>"
            << "\t" << "<max>"
            << "\t" << "<n>"
            << std::endl;
        for (auto& _ : idx) {
            const Track& track = tracks.find(_.first)->second;
            stream
                << std::setw(maxlen) << _.first
                << "\t" << (track.n == 0 ? 0 : (track.sum / track.n))
                << "\t" << track.min
                << "\t" << track.max
                << "\t" << track.n
                << std::endl;
        }
    }
    else
        for (auto& _ : idx) {
            const Track& track = tracks.find(_.first)->second;
            stream
                << std::setw(maxlen) << _.first
                << "\t" << (track.n == 0 ? 0 : (track.sum / track.n))
                << std::endl;
        }
}
