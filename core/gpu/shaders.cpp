#include "shaders.h"
#include "pipeline.h"
#include "bgl.h"

using namespace Beatmup;

#define STRINGIFY(A) #A

#ifdef BEATMUP_OPENGLVERSION_GLES
#undef BGL_SHADER_HEADER_VERSION
#define BGL_SHADER_HEADER_VERSION ""
#endif

const char
	*GL::ShaderSources::VERTEXSHADER_BLEND = BGL_SHADER_CODE(
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


	*GL::ShaderSources::VERTEXSHADER_BLENDMASK = BGL_SHADER_CODE(
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
			texCoord = (invImgMapping * vec3(inVertex, 1)).xy;									// image texture coordinates
			maskCoord = inTexCoord;
		}
	),


	*GL::ShaderSources::FRAGMENTSHADER_BLEND = STRINGIFY(
		uniform mediump vec4 modulationColor;
		varying mediump vec2 texCoord;
		void main() {
			gl_FragColor = texture2D(image, texCoord.xy).rgba * modulationColor;
		}
	),


	*GL::ShaderSources::FRAGMENTSHADER_BLENDMASK = STRINGIFY(
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
	STRINGIFY(
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
	STRINGIFY(
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


	*GL::ShaderSources::FRAGMENTSHADER_BLENDSHAPE = STRINGIFY(
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


	*GL::ShaderSources::FRAGMENTSHADERHEADER_NORMAL = BGL_SHADER_CODE(
		uniform sampler2D image;
	),


	*GL::ShaderSources::FRAGMENTSHADERHEADER_EXT = BGL_SHADER_HEADER_VERSION "\n"
		"#ifdef GL_ES\n"
		"#extension GL_OES_EGL_image_external : require\n"
		"uniform samplerExternalOES image;\n"
		"#else\n"
		"uniform sampler2D image;\n"
		"#endif\n";