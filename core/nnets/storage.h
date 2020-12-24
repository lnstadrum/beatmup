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
#include "../basic_types.h"
#include "../memory.h"
#include "../bitmap/internal_bitmap.h"
#include "../gpu/pipeline.h"
#include "../gpu/program.h"
#include "../gpu/texture_handler.h"
#include "../utils/fixed_point.h"
#include "../utils/string_utils.h"
#include <vector>


namespace Beatmup {
    namespace NNets {
        /**
            Operation 3D input/output size.
            Dimensions are X (width), Y (height), Z (depth).
        */
        class Size {
        private:
            int dim[3];

        public:
            /**
                Zero padding specification
            */
            enum class Padding {
                SAME,   //!< operation output size matches its input size for unit strides
                VALID   //!< no zero padding
            };

            static const Size EMPTY;
            static const Size ONES;

            inline Size() { dim[0] = dim[1] = dim[2] = 0; }
            inline Size(const Size& size, int depth) : Size(size[0], size[1], depth) {}
            Size(int width, int height, int depth);

            inline bool operator==(const Size& size) const {
                return (dim[0] == size.dim[0]) && (dim[1] == size.dim[1]) && (dim[2] == size.dim[2]);
            }

            inline bool operator!=(const Size& size) const {
                return (dim[0] != size.dim[0]) || (dim[1] != size.dim[1]) || (dim[2] != size.dim[2]);
            }

            inline int& operator[](int di) {
                BEATMUP_ASSERT_DEBUG(0 <= di && di < 3);
                return dim[di];
            }

            inline int operator[](int di) const {
                BEATMUP_ASSERT_DEBUG(0 <= di && di < 3);
                return dim[di];
            }

            inline int getWidth()  const { return dim[0]; }
            inline int getHeight() const { return dim[1]; }
            inline int getDepth()  const { return dim[2]; }

            inline int volume() const {
                return dim[0] * dim[1] * dim[2];
            }

            inline bool zero() const {
                return dim[0] == 0 || dim[1] == 0 || dim[2] == 0;
            }

            /**
                Computes operation output size in function of operation kernel, padding, stride and depth,
                assuming that the current Size is the input size.
                \param[in] kernel     The operation kernel size
                \param[in] stride     The stride
                \param[in] padding    The padding
                \param[in] depth      The output depth. If zero, the input depth is taken.
                \return the output size.
            */
            Size transform(Size kernel, Size stride, Padding padding, int depth = 0) const;

            /**
                Computes operation origin in function of operation kernel, padding and stride, assuming that the current
                Size instance is the input size.
                \param[in] kernel     The operation kernel size
                \param[in] stride     The stride
                \param[in] padding    The padding
                \return the origin.
            */
            Size getOrigin(Size kernel, Size stride, Padding padding) const;

            Size operator+(const Size& size) const { return Size(dim[0] + size[0], dim[1] + size[1], dim[2] + size[2]); }
            Size operator-(const Size& size) const { return Size(dim[0] - size[0], dim[1] - size[1], dim[2] - size[2]); }
            Size operator*(const Size& size) const { return Size(dim[0] * size[0], dim[1] * size[1], dim[2] * size[2]); }
            Size operator/(const Size& size) const { return Size(dim[0] / size[0], dim[1] / size[1], dim[2] / size[2]); }
            Size operator+(int scalar) const { return Size(dim[0] + scalar, dim[1] + scalar, dim[2] + scalar); }
            Size operator-(int scalar) const { return Size(dim[0] - scalar, dim[1] - scalar, dim[2] - scalar); }
            Size operator*(int scalar) const { return Size(dim[0] * scalar, dim[1] * scalar, dim[2] * scalar); }
            Size operator/(int scalar) const { return Size(dim[0] / scalar, dim[1] / scalar, dim[2] / scalar); }
        };

        /**
            3D tensor stored in a set of textures.
            Constraints:
             * The tensor entries are in [0, 1] range and sampled in 8 bits.
             * The tensor depth is a multiple of 4.
            Feature maps are stored as RGBA textures (at least four channels per texture). More feature maps can be packed in the same texture.
            The textures are all of the same size though.
        */
        class Storage : public Beatmup::Object {
        public:
            class Binder;
            class View;
            class Scanner;
            class TextureHandler;

        private:
            friend class Binder;
            friend class View;
            Storage(const Storage&) = delete;    //!< disabling copying constructor

            typedef struct {
                GL::handle_t handle;
                bool dirty;         //!< if `true`, the texture needs to be cleared before use
            } Texture;

            Context& context;
            Texture* textures;
            AlignedMemory memory;   //!< data storage in RAM
            const Size size;
            const int pad;          //!< padding in pixels added along width and height dimensions
            int packX, packY;       //!< number of blocks of 4 channels per texture (spatial packing)
            bool upToDate[2];

            void push(GraphicPipeline& gpu, const void* data);

        public:
            /**
                Creates a storage.
                \param[in] ctx                  A context
                \param[in] gpu                  A graphic pipeline instance
                \param[in] size                 Storage size
                \param[in] pad                  Additional padding to add to the texture dimensions
                \param[in] reservedChannels     Number of depth dimensions that may be sampled together with this storage
            */
            Storage(Context& ctx, GraphicPipeline& gpu, const Size size, const int pad, const int reservedChannels = 0);

            /**
                Creates a flat storage.
                It uses only one texture stacking all the feature maps in a column and has no padding.
                \param[in] ctx          A context
                \param[in] gpu          A graphic pipeline instance
                \param[in] size         Storage size
            */
            Storage(Context& ctx, GraphicPipeline& gpu, const Size size);

            ~Storage();

            /**
                Allocates the storage in GPU memory.
                \param[in] gpu    A graphic pipeline instance
            */
            void allocate(GraphicPipeline& gpu);

            /**
                Allocates the storage in RAM.
            */
            void allocate();

            /**
                Frees the allocated memory immediately.
                \param[in] gpu    A graphic pipeline instance
            */
            void free(GraphicPipeline& gpu);

            /**
                Deferred storage disposal: the textures are put into the GPU recycle bin associated with the context.
            */
            void free();

            /**
                Pulls storage data from GPU memory to RAM
                \param[in] gpu    A graphic pipeline instance
            */
            void pull(GraphicPipeline& gpu);

            /**
                Pushes storage data from RAM to CPU memory
                \param[in] gpu    A graphic pipeline instance
            */
            void push(GraphicPipeline& gpu);

            /**
                \brief Pushes a given data to GPU memory.
                The data is stored as a 3D array of (height, width, channels) layout (n and n+1 channel values are next to each other in memory).
                If the array length does not match the storage capacity, an exception is thrown.
                \param[in] gpu          A graphic pipeline instance
                \param[in] hwcData      The data to push.
                \param[in] numSamples   The data array length (number of floating point values to push).
            */
            void push(GraphicPipeline& gpu, const float* hwcData, const size_t numSamples);

            /**
                Checks if the storage is up to date for a given processing target (CPU or GPU).
                \param[in] target       The target (CPU or GPU)
                \return `true` if the data is up-to-date.
            */
            inline bool isUpToDate(ProcessingTarget target) const { return upToDate[target]; }

            /**
                Returns `true` if the storage is allocated
            */
            inline bool isAllocated() const { return textures != nullptr || memory; }

            /**
                Converts a feature channel into a bitmap for debugging purposes.
                \param[in] ctx        A context
                \param[in] gpu        A graphic pipeline instance
                \param[in] channel    Feature channel number to export
            */
            InternalBitmap* getImage(Context& ctx, GraphicPipeline& gpu, int channel) const;

            /**
                Returns total number of textures in the storage.
            */
            int getNumberOfTextures() const;

            /**
                Returns number of texture containing a given channel.
            */
            int getChannelTextureNumber(int channel) const;

            /**
                Returns origin in pixels of a given channel within the texture containing it.
            */
            IntPoint getChannelOrigin(int channel) const;

            /**
                Returns width in pixels of all the textures.
            */
            int getTextureWidth() const;

            /**
                Returns height in pixels of all the textures.
            */
            int getTextureHeight() const;

            /**
                Returns storage padding.
                Padding is zero-valued pixels on each side of the storage.
            */
            inline int getPadding() const { return pad; }

            /**
                Returns storage size in pixels.
            */
            inline Size getSize() const { return size; }

            /*
                Returns storage memory size in bytes.
                If the storage is allocated on GPU and on CPU at the same time, the used memory size is twice the returned value.
            */
            inline size_t getMemorySize() const { return (size_t)getTextureWidth() * getTextureHeight() * getNumberOfTextures() * 4; }

            /**
                Returns the storage memory, if allocated in RAM (for CPU).
            */
            inline AlignedMemory& getMemory() { return memory; }
            inline const AlignedMemory& getMemory() const { return memory; }

            /**
                Checks whether a channel number points to the first channel in a texture. Throws an exception otherwise.
            */
            inline static void checkChannelNumber(int channel) {
                class InvalidChannelNumber : public Exception {
                public:
                    InvalidChannelNumber(int channel): Exception("Invalid channel number / number of channels: %d", channel) {}
                };
                if (channel % 4 != 0)
                    throw InvalidChannelNumber(channel);
            }

            class InconsistentStorageState : public Exception {
            public:
                InconsistentStorageState(const char* message): Exception(message) {}
            };

            /**
                Maps a 3D tensor onto a storage.
                Set of storage slices along the depth dimension.
            */
            class View {
                friend class Binder;
                friend class Scanner;
                friend class TextureHandler;
            private:
                typedef struct {
                    int channelIdx;     //!< channel number in its corresponding storage
                    int textureIdx;     //!< texture number in the current view
                } Channel;

                std::vector<Channel> channels;      //!< channels of the view
                std::vector<int> textures;          //!< indices of textures in the storage
                Storage* storage;

            public:
                View(): storage(nullptr) {}
                View(View&&);
                View(const View&);
                View& operator=(View&&);

                View(Storage& storage);

                /**
                    Creates a slice of another view.
                    \param[in] view             The input storage view
                    \param[in] firstChannel     First view channel index in the storage
                    \param[in] numChannels      Number of channels in the view
                */
                View(View&& view, const int firstChannel, const int numChannels);

                /**
                    \brief Creates a view by shuffling storage channels.
                    For shuffling step n, the view will contain the storage channel quads in the following order:
                      0, 1, 2, 3, 4n, 4n+1, 4n+2, 4n+3, 8n, 8n+1, 8n+2, 8n+3, ..., 4, 5, 6, 7, 4n+4, 4n+5, 4n+6, 4n+7, 8n+4, ...
                    \param[in] storage          The storage
                    \param[in] shuffleStep      Shuffling step (n)
                */
                View(Storage& storage, const int shuffleStep);

                inline Storage& getStorage() { return *storage; }
                inline const Storage& getStorage() const { return *storage; }

                InternalBitmap* getImage(Context& ctx, GraphicPipeline& gpu, int channel) const;

                /**
                    Returns total number of textures in the storage view.
                */
                inline int getNumberOfTextures() const { return (int)textures.size(); }

                /**
                    Returns number of the texture containing a given channel.
                */
                int getChannelTextureNumber(int channel) const;

                /**
                    Returns origin in pixels of a given channel within the texture containing it.
                */
                IntPoint getChannelOrigin(int channel) const;

                /**
                    Returns width in pixels of all the textures.
                */
                inline int getTextureWidth() const { return storage->getTextureWidth(); }

                /**
                    Returns height in pixels of all the textures.
                */
                inline int getTextureHeight() const  { return storage->getTextureHeight(); }

                inline IntPoint getTextureSize() const { return IntPoint(storage->getTextureWidth(), storage->getTextureHeight()); }

                /**
                    Conversion operator to a boolean expression (`true` if the view is not empty).
                */
                inline operator bool() const { return storage != nullptr; }

                inline Size getSize() const { return storage->getSize(); }

                /**
                    Returns the spatial size (width and height) of the storage in pixels.
                */
                inline IntPoint getSpatialSize() const {
                    const Size size = storage->getSize();
                    return IntPoint(size[0], size[1]);
                };

                inline int getWidth()  const { return storage->size[0]; }
                inline int getHeight() const { return storage->size[1]; }
                inline int getDepth()  const { return 4 * (int)channels.size(); }
            };

            /**
                TextureHandler representation of a pack of 4 channels from a non-empty View.
                Does not copy any data, only stores a reference to an existing texture.
            */
            class TextureHandler : public GL::TextureHandler {
            private:
                const int width, height;
                void prepare(GraphicPipeline& gpu);
            public:
                TextureHandler(const View&, int channel);
                ~TextureHandler();
                const int getWidth() const { return width; }
                const int getHeight() const { return height; }
                const int getDepth() const { return 1; }
                const GL::TextureHandler::TextureFormat getTextureFormat() const { return GL::TextureHandler::TextureFormat::RGBAx8; }
            };

            /**
                Binding of different input/output storages/texture handlers to a GLSL program
            */
            class Binder {
            private:
                GraphicPipeline& gpu;
                GL::Program* program;
                GL::handle_t outputTexture;
                int unit;
            public:
                Binder(GraphicPipeline& gpu) : gpu(gpu), program(nullptr), outputTexture(0), unit(0) {}

                /**
                    Starts binding things to a program.
                    \param[in,out] program      The program
                    \param[in,out] output       Output storage
                    \param[in] channel          Output storage channel to be filled
                    \return `true` if the program and the output texture are already bound, i.e. are the same as in previous binding.
                */
                bool begin(GL::Program& program, Storage::View& output, int channel);

                /**
                    Starts binding things to a program which renders to a bitmap.
                    \param[in,out] program      The program
                    \param[in,out] output       The output bitmap
                */
                void begin(GL::Program& program, AbstractBitmap& output);

                /**
                    Binds a storage (all of its textures) to a uniform sampler array variable.
                    \param[in] input        The storage
                    \param[in] name         The variable name
                */
                void operator()(Storage::View& input, const char* name);

                /**
                    Binds a single texture from a storage to a uniform sampler variable.
                    \param[in] input            The storage
                    \param[in] name             The variable name
                    \param[in] channel          The channel number
                */
                void operator()(Storage::View& input, const char* name, int channel);

                void operator()(GL::TextureHandler& input, const char* name);
            };

            /**
                Scans a storageview in RAM for further computations on CPU.
                A piece of an ancient civilization technology used when neural networks were still inferred with CPU.
            */
            class Scanner {
            protected:
                typedef uint8_t sample_t[4];      //!< four unsigned 8-bit scalars
                Storage::View* view;            //!< a view to sample
                sample_t** ptr;                 //!< pointers at current position per channel
                size_t ptrSize;
                sample_t* data;                 //!< the texture data pointer

            public:
                Scanner(): view(nullptr), ptr(nullptr), ptrSize(0) {}
                Scanner(Storage::View& view): Scanner() { bind(view); }
                ~Scanner();

                /**
                    Binds a view to the scanner
                */
                void bind(Storage::View& view);

                /**
                    Unbinds the current view from the scanner
                */
                void unbind();

                /**
                    Sets the pointer to a specific spatial position.
                    \param[in] x    horizontal position in pixels
                    \param[in] y    vertical position in pixels
                */
                void move(int x, int y);

                /**
                    Advances pointer by one pixel in scanline order (along the horizontal axis).
                */
                Scanner& operator++();

                /**
                    Extracts the content of feature maps at the current position.
                    Accepts iterators of floating point STL containers, e.g. std::vector<float>::begin() and std::vector<float>::end().
                    \param[in] begin        Iterator to copy to
                    \param[in] limit        Limiting iterator position. If reached, no more samples are copied.
                */
                template<typename T>
                inline void fill(T begin, T limit) {
                    size_t i = 0;
                    const float factor = 1.0f / 255;
                    for (T it = begin; it != limit && i < ptrSize; ++i) {
                        *it++ = factor * (*ptr[i])[0];
                        if (it == limit) return;
                        *it++ = factor * (*ptr[i])[1];
                        if (it == limit) return;
                        *it++ = factor * (*ptr[i])[2];
                        if (it == limit) return;
                        *it++ = factor * (*ptr[i])[3];
                    }
                }
            };
        };


        /**
            Returns a zero padding value from a string.
            The conversion is case-insensitive. Raises an exception if cannot interpret the string.
            \param[in] str          The input string.
        */
        Size::Padding paddingFromString(const std::string& str);
    }
}

namespace std {
    std::string to_string(const Beatmup::NNets::Size::Padding& padding);
}