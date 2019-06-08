#include "layer_shader.h"
#include "../gpu/program.h"
#include "../gpu/bgl.h"

#include "../debug.h"

using namespace Beatmup;

const std::string LayerShader::BEATMUP_INPUT_IMAGE_PREPROCESSOR_DIRECTIVE = "#beatmup_input_image";

bool str_replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}


LayerShader::LayerShader(Environment& env) :
	env(env),
	fragmentShaderReady(false),
	sourceCode(),
	fragmentShader(nullptr),
	program(nullptr),
	inputFormat(GL::TextureHandler::TextureFormat::RGBx8)
{}


LayerShader::~LayerShader() {
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


void LayerShader::setSourceCode(const char* sourceCode) {
	lock();
	this->sourceCode = sourceCode;
	fragmentShaderReady = false;
	unlock();
}


void LayerShader::prepare(GraphicPipeline& gpu, GL::TextureHandler* image, const AffineMapping& mapping) {
	LockGuard lock(this);
	if (sourceCode.empty())
		throw NoSource();

	// check if input format changed
	if (image && image->getTextureFormat() != inputFormat)
		fragmentShaderReady = false;

	// compile fragment shader
	if (!fragmentShaderReady && fragmentShader) {
		delete fragmentShader;
		fragmentShader = nullptr;
	}

	if (!fragmentShader) {
		std::string preprocessed(sourceCode);
		if (image) {
			switch (inputFormat = image->getTextureFormat()) {
			case GL::TextureHandler::TextureFormat::Rx8:
			case GL::TextureHandler::TextureFormat::RGBx8:
			case GL::TextureHandler::TextureFormat::RGBAx8:
			case GL::TextureHandler::TextureFormat::Rx32f:
			case GL::TextureHandler::TextureFormat::RGBx32f:
			case GL::TextureHandler::TextureFormat::RGBAx32f:
				str_replace(preprocessed, BEATMUP_INPUT_IMAGE_PREPROCESSOR_DIRECTIVE, "uniform sampler2D");
				break;
			case GL::TextureHandler::TextureFormat::OES_Ext:
				str_replace(
					preprocessed, BEATMUP_INPUT_IMAGE_PREPROCESSOR_DIRECTIVE,
					"#extension GL_OES_EGL_image_external : require\nuniform samplerExternalOES"
				);
				break;
			default:
				throw UnsupportedInputTextureFormat(inputFormat);
			}
		}
		fragmentShader = new GL::FragmentShader(gpu, preprocessed);
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

	// enable program and configure
	gpu.getRenderingPrograms().enableProgram(&gpu, *program);
	apply(*program);

	if (image) {
		gpu.bind(*image, 0, false);
		program->setInteger("image", 0);

		AffineMapping arMapping(mapping);
		arMapping.matrix.scale(1.0f, image->getInvAspectRatio());
		program->setMatrix3("modelview", arMapping);
	}
}
