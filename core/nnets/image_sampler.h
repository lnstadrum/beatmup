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
#include "operation.h"

namespace Beatmup {
    namespace NNets {

        /**
            Image preprocessing operation.
            Samples an image of a fixed size from an arbitrary size texture. Has three key missions.
             * If enabled, performs a center crop keeping the output aspect ratio (otherwise the input is stretched to fit the output).
             * If enabled, uses linear interpolation when possible to reduce aliasing (otherwise nearest neighbor sampling is used).
             * Brings support of OES textures. This allows for example to read data directly from camera in Android.
        */
        class ImageSampler : public AbstractOperation {
        private:
            const IntPoint size;
            GL::TextureHandler *input, *output;
            GL::RenderingProgram* program;
            bool linearInterpolation;       //!< if `true`, the input image is linearly interpolated when possible
            bool centerCrop;                /**< if `true`, a center crop is performed to sample the output image from the input;
                                                 otherwise the input is stretched to match the output shape */

            int rotation;                   //!< clockwise rotation to apply to the input image; 1 unit = 90 degrees

            void prepare(GraphicPipeline& gpu, ChunkCollection& data, GL::ProgramBank& bank);
            void execute(TaskThread& thread, GraphicPipeline& gpu);
            inline int getInputPadding(int index = 0) const { return 0; }
            inline void getSampledChannels(int index, int& min, int& max) const { min = max = 0; }

        public:
            /**
                Creates an instance of image preprocessing operation.
                \param[in] name             Layer name
                \param[in] size             Output image size in pixels
                \param[in] centerCrop       If `true`, the center crop is enabled
                \param[in] linearInterp     If `true`, the linear interpolation is enabled
            */
            ImageSampler(
                const std::string& name,
                const IntPoint& size,
                bool centerCrop = true,
                bool linearInterp = true
            );

            /**
                Enables / disables center crop.
            */
            inline void setCenterCrop(bool enable) { this->centerCrop = enable; }
            inline bool getCenterCrop() const { return this->centerCrop; }

            /**
                Enables / disables linear interpolation.
            */
            inline void setLinearInterpolation(bool enable) { this->linearInterpolation = enable; }
            inline bool getLinearInterpolation() const { return this->linearInterpolation; }

            /**
                Specifies a rotation to apply to the input image.
                \param quarterTurns        Number of times a clockwise rotation by 90 degree is applied to the input image.
            */
            inline void setRotation(int quarterTurns) { this->rotation = quarterTurns; }

            /**
                Returns rotation applied to the input image.
                \return number of times a clockwise rotation by 90 degree is applied to the input image.
            */
            inline int getRotation() const { return this->rotation; }

            inline int getInputCount()  const { return 1; }
            inline int getOutputCount() const { return 1; }

            inline bool acceptsTextureInput(int index = 0) const { return index == 0; }
            inline bool acceptsTextureOutput(int index = 0) const { return index == 0; }

            inline Size getOutputSize(int outputIndex = 0) const { return Size(size.x, size.y, 3); }

            void getOutput(GL::TextureHandler*& texture, int index = 0);

            void setInput(GL::TextureHandler& texture, int inputIndex = 0);
            void setOutput(GL::TextureHandler& texture, int outputIndex = 0);

            std::map<std::string, std::string> serialize() const;

            void disconnect();

            unsigned long countTexelFetches() const;

            /**
                Sets up deserialization of the operation.
            */
            static bool initDeserializer();
        };

        /**
            \internal
            Being declared here, this variaable ensures ImageSampler::initDeserializer() is called with inclusion of this header file.
        */
        static const bool IMAGESAMPLER_OP_DESERIALIZABLE = ImageSampler::initDeserializer();
    }
}