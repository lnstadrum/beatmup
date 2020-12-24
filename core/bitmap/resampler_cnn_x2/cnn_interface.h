/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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
#include "../internal_bitmap.h"
#include "../../shading/image_shader.h"
#include "../../gpu/pipeline.h"

namespace Beatmup {

    /**
        Interface of x2 image upsampling using a convolutional neural network on GPU.
    */
    class X2UpsamplingNetwork {
    public:
        virtual ~X2UpsamplingNetwork() {};
        virtual void process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output) = 0;
        virtual bool usesEs31Backend() const = 0;
    };

}
