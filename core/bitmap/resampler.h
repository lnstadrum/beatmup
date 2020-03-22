/*
    Bitmap format converter
*/

#include "abstract_bitmap.h"
#include "../parallelism.h"
#include "../geometry.h"

namespace Beatmup {

    class BitmapResampler : public AbstractTask {
    public:
        /**
            Specifies resampling mode
        */
        enum class Mode {
            NEAREST_NEIGHBOR,    //!< usual nearest neighbor
            BOX                  //!< anti-aliasing box filter; identical to nearest neigbor when upsampling
        };
    private:
        AbstractBitmap *input, *output;                     //!< input and output bitmaps
        IntRectangle srcRect, destRect;
        Mode mode;
        
    protected:
        virtual bool process(TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex, GraphicPipeline*);
        virtual void afterProcessing(ThreadIndex, bool);
    public:
        BitmapResampler();
        void setBitmaps(AbstractBitmap* input, AbstractBitmap* output);
        void setMode(Mode mode);
        
        void setInputRect(const IntRectangle& rect);
        void setOutputRect(const IntRectangle& rect);
        IntRectangle getInputRect() const { return srcRect; }
        IntRectangle getOutputRect() const { return destRect; }

        ThreadIndex maxAllowedThreads() const;
    };
}