/*
    2D convolution operations
*/

#pragma once
#include "operation.h"
#include "../../gpu/storage_buffer.h"
#include "../../geometry.h"
#include "../../utils/string_builder.h"
#include "../../utils/array.h"

namespace Beatmup {
	namespace NNets {
		namespace Ops{

			class Convolution2D : public Feedforward2DGPUOperation {
			public:
				enum class ActivationFunc {
					IDENTITY,
					BRELU6
				};
			private:
				enum class ComputingMode {
					INLINE,
					ARRAYS,
					BUFFERS
				};

				const ComputingMode mode;
				const ActivationFunc actFunc;
				const bool depthwise;

				GL::StorageBuffer weights, biases;
				Size localGroupSize;

				static ComputingMode chooseMode(Size kernel, int outputChannels, bool depthwise);

				const int getNumberOfSlices() const;
				Storage::Type getOutputType(int outputIndex = 0) const;
				bool setInputType(Storage::Type type, int inputIndex = 0);

				std::vector<std::string> generateCode(GraphicPipeline& gpu, ChunkFile& data);

				/**
					Prepares prefetching to shared memory optimizing the number of threads per workgroup under
					the shared memory constraint.
					\param[in] gpu					A GraphicPipeline instance
					\param[out] dataCode			The output code to write out shared buffer declaration
					\param[out] localGroup			Local workgroup size
					\param[in] minScalarZLayers		Required number of scalar layers along Z axis in the shared storage
					\param[in] maxZThreads			Maximum number of threads along Z axis
				*/
				int prepareInputPrefetching(
					const GraphicPipeline& gpu,
					StringBuilder& dataCode,
					Size& localGroup,
					int minScalarZLayers,
					int maxZThreads);

				void renderDepthwise(const GraphicPipeline& gpu, const int sliceIdx, StringBuilder& computeCode, StringBuilder& dataCode);

				void renderDepthwiseArrayBased(
					const GraphicPipeline& gpu,
					const int sliceIdx,
					StringBuilder& computeCode,
					StringBuilder& dataCode,
					Array<float, 4>& kernels,
					Array<float, 1>& biases
				);

				void renderDepthwiseInline(
					const GraphicPipeline& gpu,
					const int sliceIdx,
					StringBuilder& computeCode,
					StringBuilder& dataCode,
					Array<float, 4>& kernels,
					Array<float, 1>& biases
				);

				void renderArrayBased(
					const GraphicPipeline& gpu,
					const int sliceIdx,
					StringBuilder& computeCode,
					StringBuilder& dataCode,
					Array<float, 4>& kernels,
					Array<float, 1>& biases
				) const;

				void renderInline(
					const GraphicPipeline& gpu,
					const int sliceIdx,
					StringBuilder& computeCode,
					StringBuilder& dataCode,
					Array<float, 4>& kernels,
					Array<float, 1>& biases
				);
				
				void renderPointwise(const GraphicPipeline& gpu, StringBuilder& computeCode, StringBuilder& dataCode);

				void perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int sliceIdx) const;
			public:
				Convolution2D(
					Environment& env,
					const std::string& name,
					const Size& inputSize,
					const int outChannelsNum,
					const Size& kernelSize,
					bool depthwise = false,
					const Size& stride = Size::ONES,
					Size::Padding padding = Size::Padding::SAME,
					ActivationFunc actFunc = ActivationFunc::BRELU6
				);
				~Convolution2D();

				int getMaxProgress() const {
					return getNumberOfSlices();
				}
			};

		}
	}
}