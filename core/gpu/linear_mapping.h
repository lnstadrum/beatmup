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
#include "../gpu/pipeline.h"
#include "../gpu/program_bank.h"
#include <array>

namespace Beatmup {
    namespace GL {
        /**
            Real-valued vector usable by GPU
        */
        class Vector : public GL::TextureHandler {
        public:
            /**
                Vector data format
            */
            enum class Format {
                TEXTURE,    //!< 8 bit per element covering [0, 1] range
                FIXED16,    //!< 16 bit per element
                FLOAT,      //!< 32 bit per element, floating point
            };
        private:
            Context& context;
            const TextureFormat texFormat;  //!< texture format of the texture handler
            const Format format;            //!< data format
            const int size;                 //!< number of samples in the vector
            float mapScale, mapOffset;

            void prepare(GraphicPipeline& gpu);
        public:
            static const Format DEFAULT_FORMAT;     // default format option depending on the device capabilities (FLOAT if available, FIXED16 otherwise)

            Vector(Context& context, GraphicPipeline& gpu, const int size, const Format format);

            /**
               Creates a vector in GPU memory
               \param[in] context       A context
               \param[in] gpu           A graphic pipeline instance
               \param[in] size          Size of the vector (number of scalar dimensions)
               \param[in] format        Vector data format used to store the values in GPU memory
               \param[in] values        Vector elements values
               \param[in] remap         If `true` and a fixed-point data format is used, the stored values are remapped to fit the range of the coresponding fixed-point representation.
            */
            Vector(Context& context, GraphicPipeline& gpu, const int size, const Format format, const float* values, const bool remap = false);

            ~Vector();

            /**
                Grabs vector values back from GPU to user memory.
                \param[in] gpu           A graphic pipeline instance
                \param[in] output        The output vector
            */
            void fetch(GraphicPipeline& gpu, std::vector<float>& output) const;

            inline Format getDataFormat() const { return format; }

            /**
                Returns the length of the vector
            */
            inline int getSize() const { return size; }

            /**
                Returns memory size in bytes taken by the vector.
            */
            size_t getMemorySize() const;


            inline const int getWidth() const { return 1; }
            inline const int getHeight() const { return format == Format::FIXED16 ? size / 2 : size / 4; }
            inline const int getDepth() const { return 1; }
            inline const TextureFormat getTextureFormat() const { return texFormat; }

            /**
                \brief Provides the scale factor applied to the vector values sent to GPU.
                The scale factor is applied if remapping is enabled in the constructor and a fixed-point representation is used. The vector values are remapped to match the dynamics range
                of the fixed point representation: [gpu texture values] = a * [original values] + b
                \return the scale factor a.
            */
            inline float getMappingScale() const { return mapScale; }

            /**
                \brief Provides the constant offset added to the vector values sent to GPU.
                The offset is added if remapping is enabled in the constructor and a fixed-point representation is used. The vector values are remapped to match the dynamics range
                of the fixed point representation: [gpu texture values] = a * [original values] + b
                \return the offset value b.
            */
            inline float getMappingOffset() const { return mapOffset; }
        };


        /**
            Evaluates expression A*x + b = y for a matrix A and vectors x and b using GPU.
            b is optional.
            x is of float/texture format.
            y might be in floating point, 16 bit fixed point or texture format.
        */
        class LinearMapping {
        private:
            static const int MULT_STAGE_STEPS = 2;      //!< number of 4*4 blocks processed at the multiplication stage
            static const int SUM_STAGE_STEPS = 4;       //!< number of vectors summed per shader at the summation stage
            class Matrix;

            Context& context;
            Matrix* buffer[2];                      //!< intermediate buffers used at summation stage in a circular fashion
            Matrix* matrix;                         //!< the matrix ("A")
            Vector* bias;                           //!< optional bias vector ("b")
            RenderingProgram* multStage;            //!< multiplication stage program: small pieces of the vector are multiplied by small pieces of the matrix
            RenderingProgram* sumStage;             //!< summation stage program: the pieces are collected together (repeated multiple times)
            RenderingProgram* lastSumStage;         //!< last summation stage program: adding bias and clamping output
            ProgramBank* programBank;               //!< if not null, manages the programs
            std::array<float, 4> multStageDelta;
            std::vector<std::array<float, SUM_STAGE_STEPS>> sumStageDelta;
            Beatmup::Rectangle multStageTexCoords;
            std::vector<Beatmup::Rectangle> sumStageTexCoords;
            const int leftPadding;                  //!< zero pixels added on the left side in the buffers to sum for whatever the input size is
            const bool forceFixed16Storage;         //!< if `true`, 16 bit fixed-point storages are used even if floating point compute is supported by the GPU
            bool fixed16Input;                      //!< if `true`, the input vector `x` is stored using 16 bit fixed-point format (float or texture otherwise)
            bool fixed16Output;                     //!< if `true`, the output vector `y` is stored using 16 bit fixed-point format (float or texture otherwise)
            bool ready;

        protected:
            /**
                Prepares the mapping for application (builds its GPU programs).
                \param[in] gpu          A GraphicPipeline instance
                \param[in] output       Texture handler representing the output vector `y`
                \param[in] input        Texture handler representing the output vector `x`
                \param[in,out] bank     A program bank to store the GPU programs and share with other mappings. May be null.
            */
            void prepare(GraphicPipeline& gpu, TextureHandler& output, TextureHandler& input, ProgramBank* bank = nullptr);

            void process(GraphicPipeline& gpu, TextureHandler& output, TextureHandler& input);

        public:
            /**
                Instantiates LinearMapping.
                \param context          A context instance
                \param forceFixed16     Force 16 bit fixed-point storage.
            */
            LinearMapping(Context& context, bool forceFixed16 = false);
            ~LinearMapping();

            void setMatrix(GraphicPipeline& gpu, const int width, const int height, const float* values);
            void setBias(GraphicPipeline& gpu, const int height, const float* values);

            void operator()(GraphicPipeline& gpu, TextureHandler& result, TextureHandler& input);
        };
    }
}
