#pragma once
#include "../basic_types.h"

namespace Beatmup {
    color4i fromPackedInt(int32_t _) {
        return color4i{ (uint8_t)(_ & 0xff), (uint8_t)((_ >> 8) & 0xff), (uint8_t)(_ >> 16), (uint8_t)((_ >> 24) & 0xff) };
    }
}
