/*
    x2 image upsampling using convolutional neural network inference on GPU
*/

#pragma once
#include "../internal_bitmap.h"
#include "../../shading/image_shader.h"
#include "../../gpu/pipeline.h"

namespace Beatmup {

    class X2UpsamplingNetwork {
    public:
        virtual ~X2UpsamplingNetwork() {};
        virtual void process(GraphicPipeline& gpu, GL::TextureHandler& input, AbstractBitmap& output) = 0;
    };

}
