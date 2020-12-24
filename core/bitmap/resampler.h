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
#include "abstract_bitmap.h"
#include "../parallelism.h"
#include "../geometry.h"

namespace Beatmup {

    class X2UpsamplingNetwork;

    /**
        Resamples an image to a given resolution.
        Implements different resampling approaches, including standard ones (bilinear, bicubic, etc.) and a neural network-based 2x upsampling
        approach dubbed as "x2".
    */
    class BitmapResampler : public AbstractTask, private BitmapContentLock {
    public:
        /**
            Resampling mode (algorithm) specification.
        */
        enum class Mode {
            NEAREST_NEIGHBOR,    //!< zero-order: usual nearest neighbor
            BOX,                 //!< "0.5-order": anti-aliasing box filter; identical to nearest neighbor when upsampling
            LINEAR,              //!< first order: bilinear interpolation
            CUBIC,               //!< third order: bicubic interpolation
            CONVNET              //!< upsampling x2 using a convolutional neural network
        };
    private:
        Context& context;                  //!< a context managing intermediate bitmaps
        AbstractBitmap *input, *output;    //!< input and output bitmaps
        IntRectangle srcRect, destRect;
        Mode mode;
        float cubicParameter;
        X2UpsamplingNetwork* convnet;      //!< convnet instance
        bool isUsingEs31IfAvailable;       //!< if `true`, uses OpenGL ES 3.1 backend when available instead ES 2.0

    protected:
        virtual TaskDeviceRequirement getUsedDevices() const;
        virtual bool process(TaskThread& thread);
        virtual bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex, ProcessingTarget target, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, GraphicPipeline*, bool);
        virtual ThreadIndex getMaxThreads() const;

    public:
        static const float DEFAULT_CUBIC_PARAMETER;

        /**
            Creates a resampler.
            \param context      A context instance used to manage resources required to run some of resampling algorithms.
        */
        BitmapResampler(Context& context);

        ~BitmapResampler();

        /**
            Sets the image to process.
            \param[in] input        The input image
        */
        void setInput(AbstractBitmap* input);

        /**
            Sets the output image.
            The output image contains the resampled version of the input one once the task finishes.
            \param[in] output       The output image
        */
        void setOutput(AbstractBitmap* output);

        inline AbstractBitmap* getInput() { return input; }
        inline AbstractBitmap* getOutput() { return output; }

        /**
            Sets the resampling algorithm to use.
            \param[in] mode     The algorithm to use
        */
        void setMode(Mode mode);

        /**
            Returns currently selected resampling algorithm.
        */
        Mode getMode() const { return mode; }

        /**
            Sets cubic interpolation parameter ("alpha").
            Has no impact if the resampling mode is different from cubic.
            \param[in] alpha    The alpha parameter value
        */
        void setCubicParameter(float alpha);

        /**
            Returns cubic interpolation parameter ("alpha").
        */
        float getCubicParameter() const { return cubicParameter; }

        /**
            Defines OpenGL ES backend selection policy (2.0 vs 3.1) when applicable.
            \param[in] useEs31    If `true`, ES 3.1 backend will be used when available, otherwise ES 2.0 is used.
        */
        inline void setUsingEs31IfAvailable(bool useEs31) { isUsingEs31IfAvailable = useEs31; }

        /**
            Specifies a rectangular working area in the input bitmap.
            Pixels outside of this area are not used.
        */
        void setInputRect(const IntRectangle& rect);
        
        /**
            Specifies a rectangular working area in the output bitmap.
            Pixels outside of this area are not affected.
        */
        void setOutputRect(const IntRectangle& rect);

        IntRectangle getInputRect() const { return srcRect; }
        IntRectangle getOutputRect() const { return destRect; }
    };
}
