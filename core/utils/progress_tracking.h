/**
    Progress tracking
*/

#pragma once
#include <stdint.h>

namespace Beatmup {
    class ProgressTracking {
    private:
        unsigned int progress;
    public:
        static ProgressTracking DEVNULL;

        ProgressTracking() : progress(0) {}

        inline void operator()() { progress++; }

        inline unsigned int getProgress() const { return progress; }

        inline void reset() { progress = 0; }
    };

}
