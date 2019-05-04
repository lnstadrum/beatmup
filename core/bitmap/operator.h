/*
	Basic binary operations on bitmap
*/

#include "abstract_bitmap.h"
#include "../parallelism.h"
#include "../geometry.h"

namespace Beatmup {

	class BitmapBinaryOperation : public AbstractTask {
	public:
		enum class Operation {
			NONE,
			ADD,
			MULTIPLY
		};

	private:
		const int MIN_PIXEL_COUNT_PER_THREAD = 1000;		//!< minimum number of pixels per worker

		AbstractBitmap *op1, *op2, *output;						//!< input and output bitmaps
		Operation operation;
		IntPoint op1Origin, op2Origin, outputOrigin;
		int cropWidth, cropHeight;

	protected:
		virtual bool process(TaskThread& thread);
		virtual void beforeProcessing(ThreadIndex, GraphicPipeline*);
		virtual void afterProcessing(ThreadIndex, bool);

	public:
		BitmapBinaryOperation();
		void setOperand1(AbstractBitmap* op1);
		void setOperand2(AbstractBitmap* op2);
		void setOutput(AbstractBitmap* output);
		void setOperation(const Operation operation);
		void setCropSize(int width, int height);
		void setOp1Origin(const IntPoint origin);
		void setOp2Origin(const IntPoint origin);
		void setOutputOrigin(const IntPoint origin);

		void resetCrop();

        int getCropWidth()  const { return cropWidth; }
        int getCropHeight() const { return cropHeight; }
        const IntPoint& getOp1Origin() const    { return op1Origin; }
        const IntPoint& getOp2Origin() const    { return op2Origin; }
        const IntPoint& getOutputOrigin() const { return outputOrigin; }

		ThreadIndex maxAllowedThreads() const;
	};
}