/*
    Bitmap format converter
*/

#pragma once
#include "abstract_bitmap.h"
#include "../parallelism.h"
#include "../geometry.h"

namespace Beatmup {

    class X2UpsamplingNetwork;

    class BitmapResampler : public AbstractTask {
    public:
        /**
            Specifies resampling mode
        */
        enum class Mode {
            NEAREST_NEIGHBOR,    //!< zero-order: usual nearest neighbor
            BOX,                 //!< "0.5-order": anti-aliasing box filter; identical to nearest neigbor when upsampling
            LINEAR,              //!< first order: bilinear interpolation
            CUBIC,               //!< third order: bicubic interpolation
            CONVNET              //!< upsampling x2 using a convolutional neural network
        };
    private:
        AbstractBitmap *input, *output;    //!< input and output bitmaps
        IntRectangle srcRect, destRect;
        Mode mode;
        float cubicParameter;
        X2UpsamplingNetwork* convnet;      //!< convnet instance
        bool isUsingEs31IfAvailable;       //!< if `true`, uses OpenGL ES 3.1 backend when available instead ES 2.0

    protected:
        virtual ExecutionTarget getExecutionTarget() const;
        virtual bool process(TaskThread& thread);
        virtual bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, GraphicPipeline*, bool);

    public:
        static const float DEFAULT_CUBIC_PARAMETER;

        BitmapResampler();
        ~BitmapResampler();

        void setBitmaps(AbstractBitmap* input, AbstractBitmap* output);
        void setMode(Mode mode);
        Mode getMode() const { return mode; }

        /**
            Sets cubic interpolation parameter ("alpha"). Has no impact if the mode is different from cubic.
            \param[in] alpha    The alpha parameter value
        */
        void setCubicParameter(float alpha);
        float getCubicParameter() const { return cubicParameter; }

        /**
            Defines OpenGL ES backend selection policy (2.0 vs 3.1) when applicable.
            \param[in] useEs31    If `true`, ES 3.1 backend will be used when available, otherwise ES 2.0 is used.
        */
        inline void setUsingEs31IfAvailable(bool useEs31) { isUsingEs31IfAvailable = useEs31; }

        void setInputRect(const IntRectangle& rect);
        void setOutputRect(const IntRectangle& rect);
        IntRectangle getInputRect() const { return srcRect; }
        IntRectangle getOutputRect() const { return destRect; }

        ThreadIndex maxAllowedThreads() const;
    };
}
