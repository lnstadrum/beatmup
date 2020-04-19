/*
    Pooling operations implementation
*/

#pragma once
#include "operation.h"

namespace Beatmup {
    namespace NNets {
        namespace Ops {
            class Pooling : public Feedforward2DGPUOperation {
            public:
                enum class Mode {
                    AVERAGE
                };

            private:
                const Mode mode;
                std::vector<std::string> generateCode(GraphicPipeline& gpu, ChunkFile& data);
                void perform(GraphicPipeline& gpu, GL::ComputeProgram& program, const int sliceIdx) const;

            public:
                Pooling(
                    Context& ctx,
                    const std::string& name,
                    const Mode mode,
                    const Size& inputSize,
                    const int outChannelsNum,
                    const Size& kernelSize,
                    const Size& stride = Size::ONES,
                    Size::Padding padding = Size::Padding::SAME
                );
                
                bool setInputType(Storage::Type type, int inputIndex = 0);
                
                int getMaxProgress() const {
                    return 1;
                }
            };
        }
    }
}