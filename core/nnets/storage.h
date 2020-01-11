/*
    A variant type collecting different ways to store data
    and tools to manipulate
*/

#pragma once
#include "../gpu/tensor.h"
#include "../gpu/storage_buffer.h"
#include "../gpu/compute_program.h"


namespace Beatmup {
    namespace NNets {
        /**
            Operation input/output size
        */
        class Size {
        private:
            int dim[3];

        public:
            enum class Padding {
                SAME,
                VALID
            };

            static const Size ONES;

            Size() { dim[0] = dim[1] = dim[2] = 0; }
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

            inline int getWidth()	const { return dim[0]; }
            inline int getHeight()	const { return dim[1]; }
            inline int getDepth()	const { return dim[2]; }

            inline int volume() const {
                return dim[0] * dim[1] * dim[2];
            }

            inline bool zero() const {
                return dim[0] == 0 || dim[1] == 0 || dim[2] == 0;
            }

            /**
                Computes operation output size in function of operation kernel, padding, stride and depth,
                assuming that the current Size is the input size.
                \param[in] kernel		The operation kernel size
                \param[in] stride		The stride
                \param[in] padding		The padding
                \param[in] depth		If `0`, the output size along the depth dimension is
                                        computed in the same way, using stride and padding.
                                        Otherwise, the value taken itself as the output depth.
                \return the output size.
            */
            Size transform(Size kernel, Size stride, Padding padding, int depth = 0) const;

            /**
                Computes operation origin in function of operation kernel, padding and stride,
                assuming that the current Size is the input size.
                Offset is the point in the input map the kernel is centered at.
                \param[in] kernel		The operation kernel size
                \param[in] stride		The stride
                \param[in] padding		The padding
                \return the offset.
            */
            Size getOrigin(Size kernel, Size stride, Padding padding) const;

            template<int dims> std::string toString() const;

            Size operator+(const Size& size) const { return Size(dim[0] + size[0], dim[1] + size[1], dim[2] + size[2]); }
            Size operator-(const Size& size) const { return Size(dim[0] - size[0], dim[1] - size[1], dim[2] - size[2]); }
            Size operator*(const Size& size) const { return Size(dim[0] * size[0], dim[1] * size[1], dim[2] * size[2]); }
            Size operator/(const Size& size) const { return Size(dim[0] / size[0], dim[1] / size[1], dim[2] / size[2]); }
            Size operator+(int scalar) const { return Size(dim[0] + scalar, dim[1] + scalar, dim[2] + scalar); }
            Size operator-(int scalar) const { return Size(dim[0] - scalar, dim[1] - scalar, dim[2] - scalar); }
            Size operator*(int scalar) const { return Size(dim[0] * scalar, dim[1] * scalar, dim[2] * scalar); }
            Size operator/(int scalar) const { return Size(dim[0] / scalar, dim[1] / scalar, dim[2] / scalar); }
        };

        class Storage {
        public:
            /**
                Defines the storage type
            */
            enum class Type {
                TEXTURE_REFERENCE,		//!< storage is a reference to an externally managed 2D texture handler
                BUFFER_REFERENCE,		//!< storage is a reference to an externally managed storage buffer
                TENSOR,					//!< storage is a single precision floating point 3D tensor
                BUFFER_1D,				//!< storage is a single precision floating point 1D storage buffer
                TENSOR_8FP_SIGNED,		//!< storage is a 8 bit fixed point 3D tensor in [-0.5..0.5] range
                TENSOR_8FP_BRELU6,		//!< storage is a 8 bit fixed point 3D tensor in [0..6] range
                TENSOR_16FP_BRELU6		//!< storage is a 16 bit fixed point 3D tensor in [0..6] range
            };

        private:
            union {
                GL::TextureHandler* textureReference;
                GL::Tensor* tensor;
                GL::StorageBuffer* buffer;
            };

            const Type type;
            const Size size;

        public:
            class DimensionsMismatch : public Exception {
            public:
                DimensionsMismatch(Type type) : Exception("Dimensions mismatch when creating a storage.") {}
            };


            /**
                Generates source code to manage storages
            */
            class CodeGenerator {
                uint32_t typeCoverage;
                std::string code;
            public:
                enum class Access {
                    LOADING,
                    STORING,
                    LOADING_AND_STORING
                };

                CodeGenerator();

                /**
                    Main code generation utility: declares a storage variable and generates
                    associated access functions.
                    \param[in] name					Variable name to declare
                    \param[in] type					Variable type
                    \param[in] size					Storage size
                    \param[in] bindingUint			Binding unit index to use in the declaration
                    \param[in] qualifiers			Additional qualifiers
                    \param[in] access				Access kind(s) to provide
                    \param[in] accessFuncSuffix		Access functions suffix. If "Foo" is passed,
                                                    loadFoo(...) and/or storeFoo(...) are added
                                                    for reading/writing the storage respectively.
                    \return the instance of CodeGenerator used, to enchain easily.
                */
                CodeGenerator& operator()
                    (const char* name, Type type, const Size& size, int bindingUnit, const char* qualifiers,
                    Access access, const char* accessFuncSuffix = "");

                CodeGenerator& declare(const char* name, Type type, const Size& size, int bindingUnit, const char* qualifiers);
                CodeGenerator& addLoadingAccess(const char* name, const char* suffix, Type type, const Size& size);
                CodeGenerator& addStoringAccess(const char* name, const char* suffix, Type type, const Size& size);

                inline std::string get() const { return code; }
            };
            
            /**
                Creates a storage.
                \param[in] env		An environment
                \param[in] width	Storage width
                \param[in] height	Storage height
                \param[in] depth	Storage depth
                \param[in] type		Storage type
            */
            Storage(Environment& env, const int width, const int height, const int scalarDepth, const Type type);

            Storage(Environment& env, const int length);
            
            /**
                Creates a 2D storage pointing to an externally managed texture.
                \param[in] texture		The texture
            */
            Storage(GL::TextureHandler* texture);
            
            /**
                Creates a 2D storage pointing to an externally managed buffer.
                \param[in] buffer		The buffer
            */
            Storage(GL::StorageBuffer* buffer);

            ~Storage();

            /**
                Binds the storage to a binding point in a program
                \param[in] gpu				Graphic pipeline instance, the
                \param[in,out] program		The program
                \param[in] unit				The binding point
                \param[in] read				The storage meant to be read during the program execution
                \param[in] write			The storage meant to be written during the program execution
            */
            void bind(GraphicPipeline& gpu, GL::ComputeProgram& program, int unit, bool read, bool write);

            inline Type getType() const { return type; }
            inline Size getSize() const { return size; }
            inline int getWidth()  const { return size[0]; }
            inline int getHeight() const { return size[1]; }
            inline int getDepth()  const { return size[2]; }

            /**
                Implements size-type correspondence convention.
                Type representing a given data is deduced from it corresponding size.
                \param[in] size		The size.
                \return type corresponding to the given size.
            */
            inline static Type getDefaultType(const Size& size) {
                int dims =
                    (size[0] > 1 ? 1 : 0) +
                    (size[1] > 1 ? 1 : 0) +
                    (size[2] > 1 ? 1 : 0);
                if (dims == 1)
                    return Type::BUFFER_1D;
                return Type::TENSOR;
            }

            static std::string toString(Type type);

            static std::string getTypeName(Type type);
            static int getEntrySize(Type type);
        };
    }
}