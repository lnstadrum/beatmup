#include "image_shader.h"
#include "../gpu/program.h"
#include "../gpu/bgl.h"
#include "../debug.h"


using namespace Beatmup;


const std::string
    ImageShader::INPUT_IMAGE_DECL_TYPE = "beatmupInputImage",
    ImageShader::INPUT_IMAGE_ID        = "image",
    ImageShader::CODE_HEAD =
        INPUT_IMAGE_DECL_TYPE + " " + INPUT_IMAGE_ID +";\n" +
        GL::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG;


bool str_replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


static AffineMapping getOutputCropMapping(const ImageResolution& out, const IntRectangle& outputClipRect) {
    return AffineMapping(Rectangle(
        (float)outputClipRect.getX1() / out.getWidth(),
        (float)outputClipRect.getY1() / out.getHeight(),
        (float)outputClipRect.getX2() / out.getWidth(),
        (float)outputClipRect.getY2() / out.getHeight()
    )).getInverse();
}


ImageShader::ImageShader(GL::RecycleBin& recycleBin) :
    recycleBin(recycleBin),
    program(nullptr),
    upToDate(false),
    inputFormat(GL::TextureHandler::TextureFormat::RGBx8)
{}


ImageShader::ImageShader(Context& ctx) :
    ImageShader(*ctx.getGpuRecycleBin())
{}


ImageShader::~ImageShader() {
    class Deleter : public GL::RecycleBin::Item {
        GL::Program* program;
    public:
        Deleter(GL::Program* program): program(program) {};
        ~Deleter() {
             delete program;
        }
    };

    if (program)
        recycleBin.put(new Deleter(program));
}


void ImageShader::setSourceCode(const std::string& sourceCode) {
    lock();
    this->sourceCode = sourceCode;
    upToDate = false;
    unlock();
}


void ImageShader::setOutputClipping(const IntRectangle& rectangle) {
    this->outputClipRect = rectangle;
}


void ImageShader::prepare(GraphicPipeline& gpu, GL::TextureHandler* input, const TextureParam texParam, AbstractBitmap* output, const AffineMapping& mapping) {
    LockGuard lock(this);
    if (sourceCode.empty())
        throw NoSource();

    // check if the input format changes
    if (input && input->getTextureFormat() != inputFormat)
        upToDate = false;

    // make program ready if not yet or if not up to date
    if (!program || !upToDate) {
        std::string code;
        if (input) {
            switch (inputFormat = input->getTextureFormat()) {
            case GL::TextureHandler::TextureFormat::Rx8:
            case GL::TextureHandler::TextureFormat::RGBx8:
            case GL::TextureHandler::TextureFormat::RGBAx8:
            case GL::TextureHandler::TextureFormat::Rx32f:
            case GL::TextureHandler::TextureFormat::RGBx32f:
            case GL::TextureHandler::TextureFormat::RGBAx32f:
                code = BEATMUP_SHADER_HEADER_VERSION
                    "#define " + INPUT_IMAGE_DECL_TYPE + " uniform sampler2D\n" + sourceCode;
                break;
            case GL::TextureHandler::TextureFormat::OES_Ext:
                code = BEATMUP_SHADER_HEADER_VERSION
                    "#extension GL_OES_EGL_image_external : require\n"
                    "#define " + INPUT_IMAGE_DECL_TYPE + " uniform samplerExternalOES\n" + sourceCode;
                break;
            default:
                throw UnsupportedTextureFormat(inputFormat);
            }
        }
        else {
            code = BEATMUP_SHADER_HEADER_VERSION + sourceCode;
        }

        // link program
        GL::FragmentShader fragmentShader(gpu, code);
        if (!program) {
            program = new GL::Program(gpu);
            program->link(gpu.getDefaultVertexShader(), fragmentShader);
        }
        else
            program->relink(fragmentShader);

        upToDate = true;
    }

    // enable program
    gpu.getRenderingPrograms().enableProgram(&gpu, *program, input != nullptr);

    // bind output
    if (output)
        if (!outputClipRect.empty())
            gpu.bindOutput(*output, outputClipRect);
        else
            gpu.bindOutput(*output);

    // bind input
    if (input)
        gpu.bind(*input, 0, texParam);
        // Binding order matters: texture unit 0 is used for input now.

    program->setMatrix3(
        GL::RenderingPrograms::MODELVIEW_MATRIX_ID,
        !output || outputClipRect.empty() ? mapping :
          (getOutputCropMapping(output ? output->getSize() : gpu.getDisplayResolution(), outputClipRect) * mapping)
    );

    // apply bundle
    apply(*program);
}


void ImageShader::prepare(GraphicPipeline& gpu, GL::TextureHandler* input, AbstractBitmap* output) {
    prepare(gpu, input, TextureParam::INTERP_LINEAR, output, AffineMapping::IDENTITY);
}


void ImageShader::prepare(GraphicPipeline& gpu, AbstractBitmap* output) {
    LockGuard lock(this);
    if (sourceCode.empty())
        throw NoSource();

    // link program if not yet
    if (!program || !upToDate) {
        // link program
        GL::FragmentShader fragmentShader(gpu, BEATMUP_SHADER_HEADER_VERSION + sourceCode);
        if (!program) {
            program = new GL::Program(gpu);
            program->link(gpu.getDefaultVertexShader(), fragmentShader);
        }
        else
            program->relink(fragmentShader);

        upToDate = true;
    }

    // enable program
    gpu.getRenderingPrograms().enableProgram(&gpu, *program, false);

    // bind output
    if (output)
        if (!outputClipRect.empty())
            gpu.bindOutput(*output, outputClipRect);
        else
            gpu.bindOutput(*output);

    // set up mapping
    program->setMatrix3(
        GL::RenderingPrograms::MODELVIEW_MATRIX_ID,
        !output || outputClipRect.empty() ? AffineMapping::IDENTITY :
          getOutputCropMapping(output ? output->getSize() : gpu.getDisplayResolution(), outputClipRect)
    );

    // apply bundle
    apply(*program);
}


void ImageShader::bindSamplerArray(const char* uniformId, int startingUnit, int numUnits) {
    program->setIntegerArray(uniformId, startingUnit, numUnits);
}


void ImageShader::process(GraphicPipeline& gpu) {
    gpu.getRenderingPrograms().blend(false);
}
