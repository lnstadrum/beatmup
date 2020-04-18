#include "profiler.h"
#include "../exception.h"
#include <algorithm>
#include <iomanip>

using namespace Beatmup;

Profiler::Profiler() : total(0) {}

void Beatmup::Profiler::reset() {
    tracks.clear();
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