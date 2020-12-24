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

#include "../gpu/gpu_task.h"
#include "../bitmap/abstract_bitmap.h"

namespace Beatmup {
    class Swapper : public GpuTask {
    private:
        AbstractBitmap* bitmap;
        bool fromGpuToCpu;
        bool processOnGPU(GraphicPipeline& gpu, TaskThread&);

    public:
        Swapper(bool fromGpuToCpu);
        void setBitmap(AbstractBitmap&);

        /**
            Copies bitmap from GPU memory to RAM
        */
        static void pullPixels(AbstractBitmap& bitmap);

        /**
            Copies bitmap from RAM to GPU memory
        */
        static void pushPixels(AbstractBitmap& bitmap);
    };
}