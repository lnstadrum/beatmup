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
        RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG;


bool str_replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}


ImageShader::ImageShader(Environment& env) :
	env(env),
	fragmentShaderReady(false),
	sourceCode(),
	fragmentShader(nullptr),
	program(nullptr),
	inputFormat(GL::TextureHandler::TextureFormat::RGBx8)
{}


ImageShader::~ImageShader() {
	class Deleter : public GL::RecycleBin::Item {
		GL::Program* program;
		GL::FragmentShader* fragmentShader;
	public:
		Deleter(GL::Program* program, GL::FragmentShader* fragmentShader):
			program(program), fragmentShader(fragmentShader)
		{};
		
		~Deleter() {
			if (program)
				delete program;
			if (fragmentShader)
				delete fragmentShader;
		}
	};

	env.getGpuRecycleBin()->put(new Deleter(program, fragmentShader));
}


void ImageShader::setSourceCode(const char* sourceCode) {
	lock();
	this->sourceCode = sourceCode;
	fragmentShaderReady = false;
	unlock();
}


void ImageShader::prepare(GraphicPipeline& gpu, GL::TextureHandler* input, AbstractBitmap* output, const AffineMapping& mapping) {
	LockGuard lock(this);
	if (sourceCode.empty())
		throw NoSource();

	// check input format input
	if (input->getTextureFormat() != inputFormat)
		fragmentShaderReady = false;

	// destroy fragment shader if not up to date
	if (!fragmentShaderReady && fragmentShader) {
		delete fragmentShader;
		fragmentShader = nullptr;
	}

	// compile fragment shader if not up to date
	if (!fragmentShader) {
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
				throw UnsupportedInputTextureFormat(inputFormat);
			}
		}
		fragmentShader = new GL::FragmentShader(gpu, code);
	}

	// link program
	if (!program) {
		program = new GL::Program(gpu);
		program->link(gpu.getRenderingPrograms().getDefaultVertexShader(&gpu), *fragmentShader);
		fragmentShaderReady = true;
	}
	else if (!fragmentShaderReady) {
		program->relink(*fragmentShader);
		fragmentShaderReady = true;
	}

	// enable program
	gpu.getRenderingPrograms().enableProgram(&gpu, *program);
  
    // bind output
    if (output)
        gpu.bindOutput(*output);

	// bind input
	if (input) {
		gpu.bind(*input, 0, false);
    }
		// Binding order matters: texture unit 0 is used for input now.

	// set up mapping
	program->setMatrix3(RenderingPrograms::MODELVIEW_MATRIX_ID, mapping);

	// apply bundle
	apply(*program);
}


void ImageShader::process(GraphicPipeline& gpu) {
	gpu.getRenderingPrograms().blend(false);
}