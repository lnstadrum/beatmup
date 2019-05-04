/**
    Progress tracking
*/

#pragma once
#include <stdint.h>

namespace Beatmup {
	class ProgressTracking {
	private:
		size_t progress;
	public:
		static ProgressTracking DEVNULL;

		ProgressTracking() : progress(0) {}
		
		inline void operator()() { progress++; }

		inline size_t getProgress() const { return progress; }

		inline void reset() { progress = 0; }
	};

}