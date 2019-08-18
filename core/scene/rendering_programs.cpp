#include "rendering_programs.h"
#include "gpu/pipeline.h"
#include "../gpu/bgl.h"

const std::string
	Beatmup::RenderingPrograms::MODELVIEW_MATRIX_ID    = "modelview",
	Beatmup::RenderingPrograms::TEXTURE_COORDINATES_ID = "texCoord",
	Beatmup::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG = "varying highp vec2 texCoord;\n";


enum TextureUnits {
    IMAGE = 0,
    MASK,
    MASK_LOOKUP
};


static const char
	*VERTEX_SHADER_BLEND = BEATMUP_SHADER_CODE_V(
		attribute vec2 inVertex;
		attribute vec2 inTexCoord;
		uniform mat3 modelview;
		uniform bool flipVertically;
		varying vec2 texCoord;
		void main()
		{
			gl_Position = vec4(modelview * vec3(inVertex, 1), 1);
			gl_Position.x = gl_Position.x * 2.0 - 1.0;
			if (flipVertically)
				gl_Position.y = gl_Position.y * 2.0 - 1.0;
			else
				gl_Position.y = 1.0 - gl_Position.y * 2.0;
			texCoord = inTexCoord;
		}
	),


	*VERTEX_SHADER_BLENDMASK = BEATMUP_SHADER_CODE_V(
		attribute vec2 inVertex;
		attribute vec2 inTexCoord;
		uniform mat3 modelview;		// model plane in pixels -> output in pixels
		uniform bool flipVertically;
		uniform mat3 maskMapping;
		uniform mat3 invImgMapping;
		varying vec2 texCoord;
		varying vec2 maskCoord;
		void main()
		{
			gl_Position = vec4(modelview * maskMapping * vec3(inVertex, 1), 1);
			gl_Position.x = gl_Position.x * 2.0 - 1.0;
			if (flipVertically)
				gl_Position.y = gl_Position.y * 2.0 - 1.0;
			else
				gl_Position.y = 1.0 - gl_Position.y * 2.0;
			texCoord = (invImgMapping * vec3(inVertex, 1)).xy;    // image texture coordinates
			maskCoord = inTexCoord;
		}
	),


	*FRAGMENT_SHADER_BLEND = BEATMUP_SHADER_CODE(
		uniform mediump vec4 modulationColor;
		varying mediump vec2 texCoord;
		void main() {
			gl_FragColor = texture2D(image, texCoord.xy).rgba * modulationColor;
		}
	),


	*FRAGMENT_SHADER_BLENDMASK = BEATMUP_SHADER_CODE(
		uniform highp sampler2D mask;
		uniform highp sampler2D maskLookup;
		uniform highp float blockSize;
		uniform highp float pixOffset;
		uniform mediump vec4 modulationColor;
		uniform mediump vec4 bgColor;
		varying mediump vec2 texCoord;
		varying highp vec2 maskCoord;
	)
#ifdef BEATMUP_OPENGLVERSION_GLES20
	BEATMUP_SHADER_CODE(
		void main() {
			highp float o = mod(maskCoord.x, blockSize);
			highp float a = 0.0;
			if (texCoord.x >= 0.0 && texCoord.y >= 0.0 && texCoord.x < 1.0 && texCoord.y < 1.0)
				a = texture2D(
					maskLookup,
					vec2(texture2D(mask, vec2(maskCoord.x - o + pixOffset, maskCoord.y)).a, o / blockSize + 0.03125)
				).a;
			gl_FragColor = mix(bgColor, texture2D(image, texCoord.xy).rgba, a) * modulationColor;
		}
	),
#else
	BEATMUP_SHADER_CODE(
		void main() {
			highp float o = mod(maskCoord.x, blockSize);
			highp float a = 0.0;
			if (texCoord.x >= 0.0 && texCoord.y >= 0.0 && texCoord.x < 1.0 && texCoord.y < 1.0)
				a = texture2D(
					maskLookup,
					vec2(texture2D(mask, vec2(maskCoord.x - o + pixOffset, maskCoord.y)).r, o / blockSize + 0.03125)
				).a;
			gl_FragColor = mix(bgColor, texture2D(image, texCoord.xy).rgba, a) * modulationColor;
		}
	),
#endif


	*FRAGMENT_SHADER_BLENDMASK_8BIT = BEATMUP_SHADER_CODE(
		uniform highp sampler2D mask;
		uniform mediump vec4 modulationColor;
		uniform mediump vec4 bgColor;
		varying mediump vec2 texCoord;
		varying highp vec2 maskCoord;
	)
#ifdef BEATMUP_OPENGLVERSION_GLES20
	BEATMUP_SHADER_CODE(
		void main() {
			highp float a = 0.0;
			if (texCoord.x >= 0.0 && texCoord.y >= 0.0 && texCoord.x < 1.0 && texCoord.y < 1.0)
				a = texture2D(mask, maskCoord).a;
			gl_FragColor = mix(bgColor, texture2D(image, texCoord.xy).rgba, a) * modulationColor;
		}
	),
#else
	BEATMUP_SHADER_CODE(
		void main() {
			highp float a = 0.0;
			if (texCoord.x >= 0.0 && texCoord.y >= 0.0 && texCoord.x < 1.0 && texCoord.y < 1.0)
				a = texture2D(mask, maskCoord).r;
			gl_FragColor = mix(bgColor, texture2D(image, texCoord.xy).rgba, a) * modulationColor;
		}
	),
#endif


	*FRAGMENT_SHADER_BLENDSHAPE = BEATMUP_SHADER_CODE(
		varying mediump vec2 texCoord;
		varying mediump vec2 maskCoord;
		uniform highp vec2 borderProfile;
		uniform highp float slope;
		uniform highp float border;
		uniform highp float cornerRadius;
		uniform mediump vec4 modulationColor;
		uniform mediump vec4 bgColor;

		void main() {
			highp vec2 cornerCoords = vec2(cornerRadius - min(maskCoord.x, 1.0 - maskCoord.x) * borderProfile.x, cornerRadius - min(maskCoord.y, 1.0 - maskCoord.y) * borderProfile.y);
			highp float r;
			if (cornerRadius > 0.0 && cornerCoords.x > 0.0 && cornerCoords.y > 0.0)
				r = length(cornerCoords / cornerRadius) * cornerRadius;
			else
				r = max(cornerCoords.x, cornerCoords.y);
			if (texCoord.x < 0.0 || texCoord.y < 0.0 || texCoord.x >= 1.0 || texCoord.y >= 1.0)
				gl_FragColor = bgColor;
			else
				gl_FragColor = texture2D(image, texCoord.xy).rgba;
			gl_FragColor = gl_FragColor * clamp((cornerRadius - border - r) / (slope + 0.00098), 0.0, 1.0) * modulationColor;
		}
	),


	*FRAGMENT_SHADER_HEADER_NORMAL = BEATMUP_SHADER_CODE(
		uniform sampler2D image;
	),


	*FRAGMENT_SHADER_HEADER_EXT = BEATMUP_SHADER_HEADER_VERSION "\n"
		"#ifdef GL_ES\n"
		"#extension GL_OES_EGL_image_external : require\n"
		"uniform samplerExternalOES image;\n"
		"#else\n"
		"uniform sampler2D image;\n"
		"#endif\n";



static const int
	MASK_TEXTURE_UNIT = 3,
	MASK_LOOKUP_TEXTURE_UNIT = 2;


using namespace Beatmup;


class RenderingPrograms::Backend {
private:
	/**
		Vertex attribute buffer entry
	*/
	typedef struct {
		GLfloat x, y, s, t;
	} VertexAttribBufferElement;

	GLuint hMaskLookups[3];			//!< texture containing mask values for 1, 2 and 4 bpp

	VertexAttribBufferElement vertexAttribBuffer[4];
	GLuint hVertexAttribBuffer;				//!< buffer used when rendering

	bool vertexAttrBufSet, maskLookupsSet;

public:
	Backend() : maskLookupsSet(false), vertexAttrBufSet(false) {}
	~Backend() {}


	void setupVertexAttributes(GL::Program& program, float texScaleX = 1.0f, float texScaleY = 1.0f) {
		if (!vertexAttrBufSet) {
			// attribute buffer initialization
			glGenBuffers(1, &hVertexAttribBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, hVertexAttribBuffer);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			vertexAttribBuffer[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
			vertexAttribBuffer[1] = { 1.0f, 0.0f, texScaleX, 0.0f };
			vertexAttribBuffer[2] = { 0.0f, 1.0f, 0.0f,      texScaleY };
			vertexAttribBuffer[3] = { 1.0f, 1.0f, texScaleX, texScaleY };
		}

		if (vertexAttribBuffer[1].s != texScaleX || vertexAttribBuffer[3].s != texScaleX ||
			vertexAttribBuffer[2].t != texScaleY || vertexAttribBuffer[3].t != texScaleY || !vertexAttrBufSet)
		{
			vertexAttribBuffer[1].s = vertexAttribBuffer[3].s = texScaleX;
			vertexAttribBuffer[2].t = vertexAttribBuffer[3].t = texScaleY;
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertexAttribBuffer), vertexAttribBuffer, GL_DYNAMIC_DRAW);
			vertexAttrBufSet = true;
		}

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(program.getAttribLocation("inVertex"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), NULL);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(program.getAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
#ifdef BEATMUP_DEBUG
		GL::GLException::check("vertex attributes buffer setup");
#endif
	}


	void bindMaskLookup(PixelFormat format) {
		glActiveTexture(GL_TEXTURE0 + TextureUnits::MASK_LOOKUP);

		if (!maskLookupsSet) {
			// masked blending lookup textures initialization
			unsigned char mask1[8][256], mask2[4][256], mask4[2][256];
			for (int v = 0; v < 256; ++v) {
				for (int o = 0; o < 8; ++o)
					mask1[o][v] = ((v >> o) % 2) * 255;
				for (int o = 0; o < 4; ++o)
					mask2[o][v] = (int)((v >> (2 * o)) % 4) * 255 / 3;
				for (int o = 0; o < 2; ++o)
					mask4[o][v] = (int)((v >> (4 * o)) % 16) * 255 / 15;
			}
			glGenTextures(3, hMaskLookups);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glBindTexture(GL_TEXTURE_2D, hMaskLookups[0]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 256, 8, 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask1);
			glBindTexture(GL_TEXTURE_2D, hMaskLookups[1]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 256, 4, 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask2);
			glBindTexture(GL_TEXTURE_2D, hMaskLookups[2]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 256, 2, 0, GL_ALPHA, GL_UNSIGNED_BYTE, mask4);
			maskLookupsSet = true;
		}

		switch (format) {
		case BinaryMask:
			glBindTexture(GL_TEXTURE_2D, hMaskLookups[0]);
			break;
		case QuaternaryMask:
			glBindTexture(GL_TEXTURE_2D, hMaskLookups[1]);
			break;
		case HexMask:
			glBindTexture(GL_TEXTURE_2D, hMaskLookups[2]);
			break;
		default:
			throw GL::GLException("Mask bitmap pixel format is not supported");
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
};


RenderingPrograms::RenderingPrograms(GraphicPipeline* gpu):
	backend(new Backend()), currentGlProgram(nullptr)
{}


RenderingPrograms::~RenderingPrograms() {
	delete backend;
}


template<typename type> type& emplace(std::map<RenderingPrograms::Program, type>& map,
	RenderingPrograms::Program program, const GraphicPipeline* gpu, const std::string sourceCode)
{
	return map.emplace(std::piecewise_construct,
		std::forward_as_tuple(program),
		std::forward_as_tuple(*gpu, sourceCode)
	).first->second;
}


GL::VertexShader& RenderingPrograms::getVertexShader(const GraphicPipeline* gpu, Program program) {
	auto& map = vertexShaders;
	auto it = map.find(program);
	if (it != map.end())
		return it->second;

	// maps shader types to internally handled shaders
	switch (program) {
	case Program::BLEND:
	case Program::BLEND_EXT:
		return emplace(map, Program::BLEND, gpu, VERTEX_SHADER_BLEND);

	case Program::MASKED_BLEND:
	case Program::MASKED_8BIT_BLEND:
	case Program::SHAPED_BLEND:
	case Program::MASKED_BLEND_EXT:
	case Program::MASKED_8BIT_BLEND_EXT:
	case Program::SHAPED_BLEND_EXT:
		return emplace(map, Program::MASKED_BLEND, gpu, VERTEX_SHADER_BLENDMASK);

	default:
		Insanity::insanity("Invalid vertex shader type encountered");
	}
}


GL::FragmentShader& RenderingPrograms::getFragmentShader(const GraphicPipeline* gpu, Program program) {
	auto& map = fragmentShaders;
	auto it = map.find(program);
	if (it != map.end())
		return it->second;

	// maps shader types to internally handled shaders
	switch (program) {
	case Program::BLEND:
		return emplace(map, Program::BLEND, gpu, std::string(FRAGMENT_SHADER_HEADER_NORMAL) + std::string(FRAGMENT_SHADER_BLEND));

	case Program::BLEND_EXT:
		return emplace(map, Program::BLEND_EXT, gpu, std::string(FRAGMENT_SHADER_HEADER_EXT) + std::string(FRAGMENT_SHADER_BLEND));

	case Program::MASKED_BLEND:
		return emplace(map, Program::MASKED_BLEND, gpu, std::string(FRAGMENT_SHADER_HEADER_NORMAL) + std::string(FRAGMENT_SHADER_BLENDMASK));

	case Program::MASKED_BLEND_EXT:
		return emplace(map, Program::MASKED_BLEND_EXT, gpu, std::string(FRAGMENT_SHADER_HEADER_EXT) + std::string(FRAGMENT_SHADER_BLENDMASK));

	case Program::MASKED_8BIT_BLEND:
		return emplace(map, Program::MASKED_8BIT_BLEND, gpu, std::string(FRAGMENT_SHADER_HEADER_NORMAL) + std::string(FRAGMENT_SHADER_BLENDMASK_8BIT));

	case Program::MASKED_8BIT_BLEND_EXT:
		return emplace(map, Program::MASKED_8BIT_BLEND_EXT, gpu, std::string(FRAGMENT_SHADER_HEADER_EXT) + std::string(FRAGMENT_SHADER_BLENDMASK_8BIT));

	case Program::SHAPED_BLEND:
		return emplace(map, Program::SHAPED_BLEND, gpu, std::string(FRAGMENT_SHADER_HEADER_NORMAL) + std::string(FRAGMENT_SHADER_BLENDSHAPE));

	case Program::SHAPED_BLEND_EXT:
		return emplace(map, Program::SHAPED_BLEND_EXT, gpu, std::string(FRAGMENT_SHADER_HEADER_EXT) + std::string(FRAGMENT_SHADER_BLENDSHAPE));

	default:
		Insanity::insanity("Invalid vertex shader type encountered");
	}
}


GL::Program& RenderingPrograms::getProgram(const GraphicPipeline* gpu, Program program) {
	auto& map = programs;
	auto it = map.find(program);
	if (it != map.end())
		return it->second;

	GL::Program& glProgram = programs.emplace(program, *gpu).first->second;
	glProgram.link(getVertexShader(gpu, program), getFragmentShader(gpu, program));
	return glProgram;
}


void RenderingPrograms::enableProgram(GraphicPipeline* gpu, Program program) {
	GL::Program& glProgram = getProgram(gpu, program);
	currentProgram = program;
	currentGlProgram = &glProgram;
	glProgram.enable(*gpu);
	backend->setupVertexAttributes(glProgram);
	glProgram.setInteger("image", TextureUnits::IMAGE);
	switch (program) {
	case Program::MASKED_BLEND:
	case Program::MASKED_BLEND_EXT:
		glProgram.setInteger("maskLookup", TextureUnits::MASK_LOOKUP);
	case Program::MASKED_8BIT_BLEND:
	case Program::MASKED_8BIT_BLEND_EXT:
		glProgram.setInteger("mask", TextureUnits::MASK);
	}
	maskSetUp = false;
}


void RenderingPrograms::enableProgram(GraphicPipeline* gpu, GL::Program& program) {
	currentProgram = Program::CUSTOM;
	currentGlProgram = &program;
	program.enable(*gpu);
	backend->setupVertexAttributes(program);
	program.setInteger("image", TextureUnits::IMAGE);
	maskSetUp = false;
}


GL::Program& RenderingPrograms::getCurrentProgram() {
	if (!currentGlProgram)
		RuntimeError("No current program");
	return *currentGlProgram;
}


void RenderingPrograms::bindMask(GraphicPipeline* gpu, AbstractBitmap& mask) {
	GL::Program& program = getCurrentProgram();
	gpu->bind(mask, TextureUnits::MASK, TextureParam::INTERP_NEAREST);
	if (mask.getBitsPerPixel() < 8) {
		backend->bindMaskLookup(mask.getPixelFormat());
		program.setFloat("blockSize", 8.0f / mask.getBitsPerPixel() / mask.getWidth());
		program.setFloat("pixOffset", 0.5f / mask.getWidth());
	}
	maskSetUp = true;
}


void RenderingPrograms::blend(bool onScreen) {
#ifdef BEATMUP_DEBUG
	if (currentProgram == Program::MASKED_BLEND || currentProgram == Program::MASKED_BLEND_EXT)
		DebugAssertion::check(maskSetUp, "Mask was not set up in masked blending");
#endif

	getCurrentProgram().setInteger("flipVertically", onScreen ? 0 : 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#ifdef BEATMUP_DEBUG
	GL::GLException::check("blending");
#endif
}


void RenderingPrograms::paveBackground(GraphicPipeline* gpu, GL::TextureHandler& content, bool onScreen) {
	// choose program
	switch (content.getTextureFormat()) {
	case GL::TextureHandler::TextureFormat::OES_Ext:
		enableProgram(gpu, RenderingPrograms::Program::BLEND_EXT);
		break;
	default:
		enableProgram(gpu, RenderingPrograms::Program::BLEND);
		break;
	}

	// setting texture coords, bitmap size and updating buffer data in GPU
	backend->setupVertexAttributes(*currentGlProgram, (float)gpu->getOutputResolution().getWidth() / content.getWidth(), (float)gpu->getOutputResolution().getHeight() / content.getHeight());

	currentGlProgram->setMatrix3(MODELVIEW_MATRIX_ID, AffineMapping::IDENTITY);
	currentGlProgram->setVector4("modulationColor", 1.0f, 1.0f, 1.0f, 1.0f);
	gpu->bind(content, 0, TextureParam::REPEAT);
	blend(onScreen);
}


GL::VertexShader& RenderingPrograms::getDefaultVertexShader(const GraphicPipeline* gpu) {
	return getVertexShader(gpu, Program::BLEND);
}
