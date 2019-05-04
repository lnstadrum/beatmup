#include "tensor.h"
#include "bgl.h"

#ifdef BEATMUP_OPENGLVERSION_GLES20
#error Insufficient OpenGL ES version: image arrays are not available in OpenGL ES 2.0.
#endif


using namespace Beatmup;
using namespace GL;


Tensor::Tensor(Environment& env, const int width, const int height, const int scalarDepth) :
	format(RGBAx32f), arrayTexture(true),
	width(width), height(height), depth(scalarDepth / 4),
	env(env),
	allocated(false)
{
	if (scalarDepth % 4 != 0 || depth <= 0 )
		throw Exception("Unsupported scalar depth %d. Must be a positive factor of 4.", scalarDepth);
}


Tensor::Tensor(Environment& env, GraphicPipeline& gpu, const int unpackedWidth, const int height, const float* data) :
	format(RGBAx32f), arrayTexture(false),
	width(unpackedWidth / 4), height(height), depth(1),
	env(env),
	allocated(true)
{
	if (unpackedWidth % 4 != 0)
		throw Exception("Unsupported unpacked width %d. Must be a positive factor of 4.", unpackedWidth);
	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexSubImage2D(GL_TEXTURE_2D,
		0, 0, 0, width, height,
		GL_RGBA,
		GL_FLOAT,
		data);		//fixme: this clamps the input values
	GL::GLException::check("tensor allocation");
}


Tensor::~Tensor() {
	if (hasValidHandle()) {
		TextureHandler::invalidate(*env.getGPURecycleBin());
	}
}


void Tensor::prepare(GraphicPipeline& gpu) {
	const GLenum target = arrayTexture ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
	glBindTexture(target, textureHandle);
	if (!allocated) {
		BEATMUP_ASSERT_DEBUG(arrayTexture);
		glTexStorage3D(target, 1, GL::TEXTUREHANDLER_INTERNALFORMATS[format], width, height, depth);
		allocated = true;
	}

	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifdef BEATMUP_DEBUG
	GLException::check("allocating 3D texture storage");
#endif
}


void Tensor::load(GraphicPipeline& gpu, int channel, const AbstractBitmap& bitmap) {
	BEATMUP_ASSERT_DEBUG(arrayTexture);
#ifdef BEATMUP_DEBUG
	if (channel < 0 || depth <= channel)
		throw Exception("Bad channel index %d for a 3D tensor of depth %d", channel, depth);
	if (bitmap.getPixelFormat() != PixelFormat::QuadFloat)
		throw Exception("Only 4-channel floating point bitmaps might be supplied to tensor");
	if (bitmap.getWidth() != getWidth() || bitmap.getHeight() != getHeight())
		throw Exception("Bitmap size (%dx%d) does not match tensor size (%dx%d)",
			bitmap.getWidth(), bitmap.getHeight(), getWidth(), getHeight());
#endif

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, channel, width, height, 1,
		GL::TEXTUREHANDLER_INTERNALFORMATS[format], GL_FLOAT, bitmap.getData(0, 0));

#ifdef BEATMUP_DEBUG
	GLException::check("3D tensor pixel transfer");
#endif
}