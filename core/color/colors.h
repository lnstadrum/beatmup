#pragma once

#include "../basic_types.h"

namespace Beatmup{
    namespace Colors {
        static const color4i
            White               = { 255, 255, 255, 255 },
            Black               = {   0,   0,   0, 255 },
            DarkSeaGreen1       = { 193, 255, 193, 255 },
            DarkSeaGreen2       = { 180, 238, 180, 255 },
            TransparentBlack    = {   0,   0,   0,   0 },
            Zero = TransparentBlack;


        static const color4f
            TransparentBlackF    = {   0,   0,   0,   0 },
            ZeroF = TransparentBlackF;
    }
}