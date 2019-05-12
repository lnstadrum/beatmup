#include "pipeline.h"
#include "../gpu/shaders.h"
#include "../bitmap/converter.h"
#include "../bitmap/internal_bitmap.h"
#include "../debug.h"


#include "bgl.h"
#include "program.h"
#include <algorithm>
#include <map>
#include <vector>
#include <deque>
#include <mutex>

using namespace Beatmup;


/**
	\internal
	Graphic pipeline private implementation (pimpl)
*/
class GraphicPipeline::Impl {
private:
	/**
		Specification of one vertex attribute buffer element
	*/
	typedef struct {
		GLfloat x, y, s, t;
	} VertexAttribBufferElement;

	GraphicPipeline& front;

	GL::Program
		*blending, *blendingMasked, *blendingShaped,
		*blendingExt, *blendingMaskedExt, *blendingShapedExt;

	struct {
		GL::VertexShader
			*vertexPlain, *vertexMasked;
		GL::FragmentShader
			*fragmentPlain, *fragmentMasked, *fragmentShaped,
			*fragmentPlainExt, *fragmentMaskedExt, *fragmentShapedExt;
	} shaders;

	GLuint hMaskLookups[3];			//!< texture containing mask values for 1, 2 and 4 bpp

	VertexAttribBufferElement vertexAttribBuffer[4];
	GLuint hVertexAttribBuffer;				//!< buffer used when rendering

	GLuint hFrameBuffer;

	GLuint textureUnitCtr;					//!< texture unit counter for binding

	bool onScreen;							//!< if `true` on-screen rendering is enabled

	ImageResolution
		displayResolution,					//!< width and height of a display obtained when switching
		outputResolution;					//!< actual output resolution (display or bitmap)

#ifdef BEATMUP_PLATFORM_WINDOWS
	HWND hwnd;
	HGLRC hglrc;

#elif BEATMUP_PLATFORM_ANDROID
	EGLDisplay eglDisplay;
	EGLSurface
		eglSurface,				//!< currently used surface
		eglDefaultSurface;		//!< default internally managed surface
	EGLContext eglContext;
	EGLConfig eglConfig;
#endif

	struct {
		int maxWorkGroupCount[3];
		int maxWorkGroupSize[3];
		int maxTotalWorkGroupSize;
		msize maxSharedMemSize;
	} glLimits;


	/**
		\internal
		Associates a valid texture handle and mark the handler as currently used
	*/
	GLuint useTexture(GL::TextureHandler& handler) {
		if (!handler.hasValidHandle())
			glGenTextures(1, &handler.textureHandle);
		return handler.textureHandle;
	}


	unsigned int bindExtTexture(GLuint handle) {
		glActiveTexture(GL_TEXTURE0 + textureUnitCtr);
		glBindTexture(BGL_TEXTURE_TARGET, handle);
		glTexParameteri(BGL_TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(BGL_TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(BGL_TEXTURE_TARGET, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(BGL_TEXTURE_TARGET, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef BEATMUP_DEBUG
		GL::GLException::check("binding EXT texture");
#endif
		return textureUnitCtr++;
	}


	inline void doRender() {
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#ifdef BEATMUP_DEBUG
		GL::GLException::check("actual rendering");
#endif
	}


public:
	Impl(GraphicPipeline& front) : onScreen(false), front(front)
	{
#ifdef BEATMUP_PLATFORM_WINDOWS
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_SUPPORT_OPENGL;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 16;
		pfd.iLayerType = PFD_MAIN_PLANE;
		hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, "STATIC", "glctx",
			WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			0, 0, 1, 1, 0, 0, GetModuleHandle(NULL), 0);
		if (!hwnd)
			throw GL::GLException("Unable to initialize GL context");

		ShowWindow(hwnd, SW_HIDE);
		HDC hdc = GetDC(hwnd);
		int pixelFormat = ChoosePixelFormat(hdc, &pfd);
		SetPixelFormat(hdc, pixelFormat, &pfd);
		hglrc = wglCreateContext(hdc);
		wglMakeCurrent(hdc, hglrc);
		if (!wglGetCurrentContext())
			throw GL::GLException("Unable to initialize GL context");

		// init glew
		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
		if (err != GLEW_OK)
			throw GL::GLException((const char*)glewGetErrorString(err));

#elif BEATMUP_PLATFORM_ANDROID
		// Step 1 - Get the default display.
		if ((eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
			throw GL::GLException("EGL: no display", eglGetError());

		// Step 2 - Initialize EGL.
		if (!eglInitialize(eglDisplay, 0, 0))
			throw GL::GLException("EGL: initialization failed", eglGetError());

		// Step 3 - Make OpenGL ES the current API.
		eglBindAPI(EGL_OPENGL_ES_API);

		// Step 4 - Specify the required configuration attributes.
		EGLint configAttributes[] = {
			EGL_SURFACE_TYPE,			EGL_PBUFFER_BIT,
#ifdef BEATMUP_OPENGLVERSION_GLES20
            EGL_RENDERABLE_TYPE,		EGL_OPENGL_ES2_BIT,
#elif defined BEATMUP_OPENGLVERSION_GLES31
			EGL_RENDERABLE_TYPE,		EGL_OPENGL_ES3_BIT_KHR,
#endif
			/*EGL_CONFORMANT,				EGL_OPENGL_ES2_BIT,
			EGL_COLOR_BUFFER_TYPE,		EGL_RGB_BUFFER,
			EGL_BIND_TO_TEXTURE_RGBA,	EGL_TRUE,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 0,
			EGL_STENCIL_SIZE, 0,*/
			EGL_NONE
		};
		int numConfigs;
		if (! eglChooseConfig(eglDisplay, configAttributes, &eglConfig, 1, &numConfigs))
			throw GL::GLException("EGL: bad configuration", eglGetError());
		DEBUG_I("Number of EGL configs got: %d", numConfigs);


		// Step 6 - Create a context.
		EGLint contextAttributes[] = {
                EGL_CONTEXT_CLIENT_VERSION,
#ifdef BEATMUP_OPENGLVERSION_GLES20
                2,
#elif defined BEATMUP_OPENGLVERSION_GLES31
                3,
#endif
                EGL_NONE };
		eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttributes);
		if (eglContext == EGL_NO_CONTEXT)
			throw GL::GLException("EGL: context initialization failed", eglGetError());

		eglDefaultSurface = eglSurface = EGL_NO_SURFACE;

		// Step 5 - Create a surface to draw to.
		EGLint surfaceAttributes[] = {
			EGL_WIDTH, 2,		// eglMakeCurrent fails sometimes with zero sizes
			EGL_HEIGHT, 2,
			EGL_NONE
		};

		eglDefaultSurface = eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, surfaceAttributes);
		if (eglSurface == EGL_NO_SURFACE)
			throw GL::GLException("EGL: window surface creation failed when init", eglGetError());

		// Step 7 - Bind the context to the current thread
		if (! eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
			throw GL::GLException("EGL: making current", eglGetError());
#else
		Undefined OS!
#endif

		// query GL limits
#if defined(BEATMUP_OPENGLVERSION_GLES31) || defined(BEATMUP_PLATFORM_WINDOWS)
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, glLimits.maxWorkGroupCount + 0);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, glLimits.maxWorkGroupCount + 1);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, glLimits.maxWorkGroupCount + 2);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, glLimits.maxWorkGroupSize + 0);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, glLimits.maxWorkGroupSize + 1);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, glLimits.maxWorkGroupSize + 2);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &glLimits.maxTotalWorkGroupSize);
		glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, (GLint*)&glLimits.maxSharedMemSize);
#ifdef BEATMUP_DEBUG
		const char
			*vendor   = (char*)glGetString(GL_VENDOR),
			*renderer = (char*)glGetString(GL_RENDERER);
		DEBUG_I("__________________________________________________________");
		DEBUG_I("Beatmup GL startup: %s / %s", renderer, vendor);
		DEBUG_I(" - Max workgroups: %d, %d, %d", glLimits.maxWorkGroupCount[0], glLimits.maxWorkGroupCount[1], glLimits.maxWorkGroupCount[2]);
		DEBUG_I(" - Max local groups: %d, %d, %d / %d", glLimits.maxWorkGroupSize[0], glLimits.maxWorkGroupSize[1], glLimits.maxWorkGroupSize[2],
			glLimits.maxTotalWorkGroupSize);
		DEBUG_I(" - Shared memory: %d KB", glLimits.maxSharedMemSize / 1024);
		DEBUG_I("__________________________________________________________");
#endif
#else
		glLimits.maxWorkGroupCount[0] = glLimits.maxWorkGroupCount[1] = glLimits.maxWorkGroupCount[2] = 0;
		glLimits.maxWorkGroupSize[0] = glLimits.maxWorkGroupSize[1] = glLimits.maxWorkGroupSize[2] = 0;
		glLimits.maxTotalWorkGroupSize = 0;
		glLimits.maxSharedMemSize = 0;
#endif

		// init buffers
		glGenFramebuffers(1, &hFrameBuffer);
		GL::GLException::check("initialization");

		shaders.vertexPlain		= new GL::VertexShader(front, GL::ShaderSources::VERTEXSHADER_BLEND);
		shaders.vertexMasked	= new GL::VertexShader(front, GL::ShaderSources::VERTEXSHADER_BLENDMASK);
		shaders.fragmentPlain		= new GL::FragmentShader(front, std::string(GL::ShaderSources::FRAGMENTSHADERHEADER_NORMAL) + std::string(GL::ShaderSources::FRAGMENTSHADER_BLEND));
		shaders.fragmentMasked		= new GL::FragmentShader(front, std::string(GL::ShaderSources::FRAGMENTSHADERHEADER_NORMAL) + std::string(GL::ShaderSources::FRAGMENTSHADER_BLENDMASK));
		shaders.fragmentShaped		= new GL::FragmentShader(front, std::string(GL::ShaderSources::FRAGMENTSHADERHEADER_NORMAL) + std::string(GL::ShaderSources::FRAGMENTSHADER_BLENDSHAPE));
		shaders.fragmentPlainExt	= new GL::FragmentShader(front, std::string(GL::ShaderSources::FRAGMENTSHADERHEADER_EXT) + std::string(GL::ShaderSources::FRAGMENTSHADER_BLEND));
		shaders.fragmentMaskedExt	= new GL::FragmentShader(front, std::string(GL::ShaderSources::FRAGMENTSHADERHEADER_EXT) + std::string(GL::ShaderSources::FRAGMENTSHADER_BLENDMASK));
		shaders.fragmentShapedExt	= new GL::FragmentShader(front, std::string(GL::ShaderSources::FRAGMENTSHADERHEADER_EXT) + std::string(GL::ShaderSources::FRAGMENTSHADER_BLENDSHAPE));
		
		// init shaders and programs
		blending       = new GL::Program(front, *shaders.vertexPlain, *shaders.fragmentPlain);
		blendingMasked = new GL::Program(front, *shaders.vertexMasked, *shaders.fragmentMasked);
		blendingShaped = new GL::Program(front, *shaders.vertexMasked, *shaders.fragmentShaped);

		blendingExt       = new GL::Program(front, *shaders.vertexPlain, *shaders.fragmentPlainExt);
		blendingMaskedExt = new GL::Program(front, *shaders.vertexMasked, *shaders.fragmentMaskedExt);
		blendingShapedExt = new GL::Program(front, *shaders.vertexMasked, *shaders.fragmentShapedExt);

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

		// attribute buffer initialization
		vertexAttribBuffer[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
		vertexAttribBuffer[1] = { 1.0f, 0.0f, 1.0f, 0.0f };
		vertexAttribBuffer[2] = { 0.0f, 1.0f, 0.0f, 1.0f };
		vertexAttribBuffer[3] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glGenBuffers(1, &hVertexAttribBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, hVertexAttribBuffer);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexAttribBuffer), vertexAttribBuffer, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(blending->getAttribLocation("inVertex"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), NULL);
		glVertexAttribPointer(blendingMasked->getAttribLocation("inVertex"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), NULL);
		glVertexAttribPointer(blendingShaped->getAttribLocation("inVertex"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), NULL);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(blending->getAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
		glVertexAttribPointer(blendingMasked->getAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
		glVertexAttribPointer(blendingShaped->getAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
		GL::GLException::check("attribute buffer initialization");

		// setting up main controls
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
		GL::GLException::check("enabling / disabling");
	}


	~Impl() {
		delete blending;
		delete blendingMasked;
		delete blendingShaped;
		delete shaders.fragmentPlain;
		delete shaders.fragmentPlainExt;
		delete shaders.fragmentMasked;
		delete shaders.fragmentMaskedExt;
		delete shaders.fragmentShaped;
		delete shaders.fragmentShapedExt;
		delete shaders.vertexMasked;
		delete shaders.vertexPlain;
		glDeleteFramebuffers(1, &hFrameBuffer);
		glDeleteBuffers(1, &hVertexAttribBuffer);

#ifdef BEATMUP_PLATFORM_WINDOWS
		wglDeleteContext(hglrc);
		DestroyWindow(hwnd);
#elif BEATMUP_PLATFORM_ANDROID
		if (eglSurface != EGL_NO_SURFACE && eglSurface != eglDefaultSurface)
			eglDestroySurface(eglDisplay, eglSurface);
		eglDestroySurface(eglDisplay, eglDefaultSurface);
		eglDestroyContext(eglDisplay, eglContext);
		eglTerminate(eglDisplay);
#endif
	}


	/**
		Platform-dependent display switching routine
	*/
	void switchDisplay(void* data) {
#ifdef BEATMUP_PLATFORM_WINDOWS
		throw GL::GLException("switchDisplay is not implemented on Windows");
#elif BEATMUP_PLATFORM_ANDROID
		// disconnecting old surface
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (eglSurface != EGL_NO_SURFACE && eglSurface != eglDefaultSurface)
			eglDestroySurface(eglDisplay, eglSurface);

		if (data) {
			// create a surface to draw to
			EGLint surfaceAttributes[] = {
				EGL_RENDER_BUFFER,  EGL_BACK_BUFFER,
				EGL_NONE
			};

			eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (ANativeWindow*)data, surfaceAttributes);
			if (eglSurface == EGL_NO_SURFACE)
				throw GL::GLException("EGL: window surface creation failed when switching display", eglGetError());

			// bind the context to the current thread
			if (! eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
				throw GL::GLException("EGL: switching display", eglGetError());

			// setting viewport
			displayResolution.set(ANativeWindow_getWidth((ANativeWindow*)data), ANativeWindow_getHeight((ANativeWindow*)data));
			glViewport(0, 0, displayResolution.getWidth(), displayResolution.getHeight());
			onScreen = true;
		}
		else {
			onScreen = false;
			eglSurface = eglDefaultSurface;
			if (! eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
				throw GL::GLException("EGL: making current", eglGetError());
			//fixme: what about display resolution?
		}
#endif

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
	}


	void swapBuffers() {
		glFinish();
#ifdef BEATMUP_PLATFORM_WINDOWS
		throw GL::GLException("swapBuffers is not implemented on Windows");
#elif BEATMUP_PLATFORM_ANDROID
		if (! eglSwapBuffers(eglDisplay, eglSurface))
			throw GL::GLException("EGL: swapping buffers", eglGetError());
#endif
	}
	

	void resetTextureBinding() {
		textureUnitCtr = 0;
	}


	unsigned int bindSampler(GL::TextureHandler& handler, const bool clampToEdge = true) {
		// setting up a new texture or taking an existing one
		glActiveTexture(GL_TEXTURE0 + textureUnitCtr);
		useTexture(handler);
		handler.prepare(front);		
		if (clampToEdge) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		GL::GLException::check("preparing texture unit");

		return textureUnitCtr++;
	}


	/**
		Binds a texture handle to an image unit
		\param[in] bitmap		The texture handler
		\param[in] unit			The image unit number
		\param[in] read			If `true`, the image will be read
		\param[in] write		If `true`, the image will be modified
		\return texture unit number
	*/
	unsigned int bindImage(GL::TextureHandler& texture, int unit, bool read, bool write) {
#ifdef BEATMUP_OPENGLVERSION_GLES20
		throw GL::Unsupported("Images binding is not supported in GL ES 2.0.");
#else
		glActiveTexture(GL_TEXTURE0 + textureUnitCtr);
		const GL::glhandle handle = useTexture(texture);
		texture.prepare(front);
		glBindImageTexture(unit,
			handle,
			0, texture.getDepth() > 1 ? GL_TRUE : GL_FALSE, 0,
			read && write ? GL_READ_WRITE : (write ? GL_WRITE_ONLY : GL_READ_ONLY),
			GL::TEXTUREHANDLER_INTERNALFORMATS[ texture.getTextureFormat() ]
		);
		GL::GLException::check("preparing image unit");
		return textureUnitCtr++;
#endif
	}

	void setInterpolation(const GraphicPipeline::Interpolation interpolation) {
		switch (interpolation) {
		case GraphicPipeline::Interpolation::NEAREST:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		case GraphicPipeline::Interpolation::LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		}
	}

	void setOutput(AbstractBitmap& bitmap) {
		if (bitmap.isMask())
			throw GL::GLException("Mask bitmaps can not be used as output");

		// setting up a texture
		glBindFramebuffer(GL_FRAMEBUFFER, hFrameBuffer);

		GLuint handle = useTexture(bitmap);
		glBindTexture(GL_TEXTURE_2D, handle);
		if (!bitmap.isUpToDate(ProcessingTarget::GPU)) {
			static const GLint formats[] = {
				0,
#ifdef BEATMUP_OPENGLVERSION_GLES20
				GL_LUMINANCE,
#else
				GL_RED,
#endif
				GL_RG, GL_RGB, GL_RGBA
			};
			glTexImage2D(
				GL_TEXTURE_2D, 0, formats[bitmap.getNumberOfChannels()],
				bitmap.getWidth(), bitmap.getHeight(),
				0,
				formats[bitmap.getNumberOfChannels()], GL::BITMAP_PIXELTYPES[bitmap.getPixelFormat()],
				NULL
			);
			GL::GLException::check("allocating output texture image");
		}

		setInterpolation(Interpolation::LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);

		GLuint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (err != GL_FRAMEBUFFER_COMPLETE)
			throw GL::GLException("framebuffer incomplete", err);

		// setting up main controls
		glViewport(0, 0, bitmap.getWidth(), bitmap.getHeight());
		outputResolution = bitmap.getImageResolution();
		glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

#ifdef BEATMUP_DEBUG
		GL::GLException::check("setting output: enabling/disabling");
#endif

		// marking bitmap as up-to-date on GPU
		bitmap.upToDate[ProcessingTarget::GPU] = true;
		onScreen = false;
	}


	void resetOutput() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, displayResolution.getWidth(), displayResolution.getHeight());
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		onScreen = true;
		outputResolution = displayResolution;
	}


	ImageResolution getOutputResolution() const {
		return outputResolution;
	}


	/**
		Transfers texture data from GPU to CPU. The bitmap is assumed to be locked.
	*/
	void fetchPixels(AbstractBitmap& bitmap) {
		if (!bitmap.upToDate[ProcessingTarget::GPU])
			return;

#ifdef BEATMUP_OPENGLVERSION_GLES20
		if (bitmap.isFloat())
			throw GL::Unsupported("Floating point valued bitmaps are not updatable from GPU memory");
				// 'cause GLES does not support such data to be transfered from GPU to CPU memory
#endif

		GLuint handle = useTexture(bitmap);
		glBindFramebuffer(GL_FRAMEBUFFER, hFrameBuffer);
		glBindTexture(GL_TEXTURE_2D, handle);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
		
		// disable high order alignment
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		bool buffered = false;
#ifdef BEATMUP_OPENGLVERSION_GLES20
		// according to GL ES 2.0 spec, only two format/type pairs may be read from GPU memory directly; otherwise we need a buffer
		if (bitmap.getPixelFormat() != QuadByte) {
			GLint format, type;
			glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
			glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
			if (GL::BITMAP_PIXELFORMATS[bitmap.getPixelFormat()] != format || GL::BITMAP_PIXELTYPES[bitmap.getPixelFormat()] != type)
				buffered = true;
		}
#endif

		// copy data with a buffer
		if (buffered) {
			InternalBitmap buffer(bitmap.getEnvironment(),
				bitmap.isFloat() ? PixelFormat::QuadFloat : PixelFormat::QuadByte,
				bitmap.getWidth(), bitmap.getHeight());
			buffer.lockPixels(ProcessingTarget::CPU);

			glReadPixels(0, 0, buffer.getWidth(), buffer.getHeight(),
				GL::BITMAP_PIXELFORMATS[buffer.getPixelFormat()],
				GL::BITMAP_PIXELTYPES[buffer.getPixelFormat()],
				buffer.getData(0, 0)
			);
			GL::GLException::check("reading pixel data from GPU memory");

			// converting to the required format
			BitmapConverter::convert(buffer, bitmap);

			buffer.unlockPixels();
		}

		// copy data directly
		else {
			glReadPixels(0, 0, bitmap.getWidth(), bitmap.getHeight(),
				GL::BITMAP_PIXELFORMATS[bitmap.getPixelFormat()],
				GL::BITMAP_PIXELTYPES[bitmap.getPixelFormat()],
				bitmap.getData(0, 0)
			);
		}

		GL::GLException::check("reading pixel data from texture");

		bitmap.upToDate[ProcessingTarget::CPU] = true;
	}


	void blend(GL::TextureHandler& image, const pixfloat4& modulation, const AffineMapping& mapping) {
		GL::Program* program;
		resetTextureBinding();
		unsigned int unit;
		switch (image.getTextureFormat()) {
		case GL::TextureHandler::TextureFormat::Rx8:
		case GL::TextureHandler::TextureFormat::RGBx8:
		case GL::TextureHandler::TextureFormat::RGBAx8:
		case GL::TextureHandler::TextureFormat::Rx32f:
		case GL::TextureHandler::TextureFormat::RGBx32f:
		case GL::TextureHandler::TextureFormat::RGBAx32f:
			program = blending;
			unit = bindSampler(image);
			break;
		case GL::TextureHandler::TextureFormat::OES_Ext:
			program = blendingExt;
			unit = bindExtTexture(image.textureHandle);
			break;
		default:
			return;
		}

		AffineMapping arMapping(mapping);
		arMapping.matrix.scale(1.0f, image.getInvAspectRatio());

		program->enable(front);
		program->setMatrix3("modelview", arMapping);
		program->setVector4("modulationColor", modulation.r, modulation.g, modulation.b, modulation.a);
		program->setInteger("flipVertically", !onScreen);
		program->setInteger("image", (int)unit);

		doRender();
	}


	void blendMasked(
		const AffineMapping& baseMapping,
		GL::TextureHandler& image,
		const AffineMapping& imageMapping,
		AbstractBitmap& mask,
		const AffineMapping& maskMapping,
		const pixfloat4& modulation,
		const pixfloat4& bgColor
	) {
		GL::Program* program = NULL;
		resetTextureBinding();
		unsigned int imageUnit, maskUnit;
		switch (image.getTextureFormat()) {
		case GL::TextureHandler::TextureFormat::Rx8:
		case GL::TextureHandler::TextureFormat::RGBx8:
		case GL::TextureHandler::TextureFormat::RGBAx8:
		case GL::TextureHandler::TextureFormat::Rx32f:
		case GL::TextureHandler::TextureFormat::RGBx32f:
		case GL::TextureHandler::TextureFormat::RGBAx32f:
			program = blendingMasked;
			imageUnit = bindSampler(image);
			break;
		case GL::TextureHandler::TextureFormat::OES_Ext:
			program = blendingMaskedExt;
			imageUnit = bindExtTexture(image.textureHandle);
			break;
		default:
			return;
		}

		AffineMapping arImgMapping(imageMapping), arMaskMapping(maskMapping);
		arImgMapping.matrix.scale(1.0f, image.getInvAspectRatio());
		arMaskMapping.matrix.scale(1.0f, image.getInvAspectRatio());

		program->enable(front);
		program->setMatrix3("modelview", baseMapping);
		program->setMatrix3("invImgMapping", arImgMapping.getInverse() * arMaskMapping);
		program->setMatrix3("maskMapping", arMaskMapping);
		program->setVector4("modulationColor", modulation.r, modulation.g, modulation.b, modulation.a);
		program->setVector4("bgColor", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
		program->setInteger("flipVertically", !onScreen);

		maskUnit = bindSampler(mask);

		// binding lookup
		const unsigned int maskHandleUnit = textureUnitCtr++;
		glActiveTexture(GL_TEXTURE0 + maskHandleUnit);
		switch (mask.getPixelFormat()) {
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
		setInterpolation(Interpolation::NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//seting up shader variables
		program->setInteger("image", (int)imageUnit);
		program->setInteger("mask", (int)maskUnit);
		program->setInteger("maskLookup", (int)maskHandleUnit);
		program->setFloat("blockSize", 8.0f / mask.getBitsPerPixel() / mask.getWidth());
		program->setFloat("pixOffset", 0.5f / mask.getWidth());

		doRender();
	}

		
	void blendShaped(
		const AffineMapping& baseMapping,
		GL::TextureHandler& image,
		const AffineMapping& imageMapping,
		const AffineMapping& maskMapping,
		const float border,
		const float slope,
		const float radius,
		const int referenceSize,
		const pixfloat4& modulation,
		const pixfloat4& bgColor
	) {
		GL::Program* program = NULL;
		resetTextureBinding();
		unsigned int unit;
		switch (image.getTextureFormat()) {
		case GL::TextureHandler::TextureFormat::Rx8:
		case GL::TextureHandler::TextureFormat::RGBx8:
		case GL::TextureHandler::TextureFormat::RGBAx8:
		case GL::TextureHandler::TextureFormat::Rx32f:
		case GL::TextureHandler::TextureFormat::RGBx32f:
		case GL::TextureHandler::TextureFormat::RGBAx32f:
			program = blendingShaped;
			unit = bindSampler(image);
			break;
		case GL::TextureHandler::TextureFormat::OES_Ext:
			program = blendingShapedExt;
			unit = bindExtTexture(image.textureHandle);
			break;
		default:
			return;
		}

		AffineMapping arImgMapping(imageMapping), arMaskMapping(maskMapping);
		arImgMapping.matrix.scale(1.0f, image.getInvAspectRatio());
		arMaskMapping.matrix.scale(1.0f, image.getInvAspectRatio());

		program->enable(front);
		program->setMatrix3("modelview", baseMapping);
		program->setMatrix3("invImgMapping", arImgMapping.getInverse() * arMaskMapping);
		program->setMatrix3("maskMapping", arMaskMapping);
		program->setVector4("bgColor", bgColor.r, bgColor.g, bgColor.b, bgColor.a);
		program->setVector4("modulationColor", modulation.r, modulation.g, modulation.b, modulation.a);
		program->setInteger("flipVertically", !onScreen);

		// computing border profile in pixels
		Matrix2 mat = baseMapping.matrix * arMaskMapping.matrix;
		mat.prescale(1.0f, outputResolution.getInvAspectRatio());
		const float scale = referenceSize > 0 ? referenceSize : 1;
		const Point borderProfile(scale * mat.getScalingX(), scale * mat.getScalingY());
		
		//seting up shader variables
		program->setInteger("image", (int)unit);
		glUniform2f(program->getUniformLocation("borderProfile"), borderProfile.x, borderProfile.y);
		program->setFloat("slope", slope);
		program->setFloat("border", border);
		program->setFloat("cornerRadius", radius + border);

		doRender();
	}


	void blendCustom(GL::TextureHandler* image, const AffineMapping& mapping, GL::Program& program) {
		resetTextureBinding();
		unsigned int unit;

		// if there is a texture, prepare it
		if (image) {
			switch (image->getTextureFormat()) {
			case GL::TextureHandler::TextureFormat::Rx8:
			case GL::TextureHandler::TextureFormat::RGBx8:
			case GL::TextureHandler::TextureFormat::RGBAx8:
			case GL::TextureHandler::TextureFormat::Rx32f:
			case GL::TextureHandler::TextureFormat::RGBx32f:
			case GL::TextureHandler::TextureFormat::RGBAx32f:
				unit = bindSampler(*image);
				break;
			case GL::TextureHandler::TextureFormat::OES_Ext:
				unit = bindExtTexture(image->textureHandle);
				break;
			}
		}

		// bind vertex attribute buffer (to be able to specify the output mapping)
#ifdef BEATMUP_DEBUG
		if (program.getVertexShader() != &getBlendingVertexShader())
			throw GL::GLException("Program vertex shader does not match default blending vertex shader");
#endif

		AffineMapping arMapping(mapping);
		arMapping.matrix.scale(1.0f, image->getInvAspectRatio());

		program.enable(front);
		program.setInteger("image", (int)unit);
		program.setMatrix3("modelview", arMapping);
		program.setInteger("flipVertically", !onScreen, true);

		doRender();
	}


	GL::VertexShader& getBlendingVertexShader() const {
		return *shaders.vertexPlain;
	}

	
	void paveBackground(AbstractBitmap& image) {
		blending->enable(front);

		// setting texture coords, bitmap size and updating buffer data in GPU
		vertexAttribBuffer[1].s = vertexAttribBuffer[3].s = (float)outputResolution.getWidth() / image.getWidth();
		vertexAttribBuffer[2].t = vertexAttribBuffer[3].t = (float)outputResolution.getHeight() / image.getHeight();
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexAttribBuffer), vertexAttribBuffer, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(blending->getAttribLocation("inVertex"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), NULL);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(blending->getAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
		
		// setting modelview matrix
		GLfloat m[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
		glUniformMatrix3fv(blending->getUniformLocation("modelview"), 1, false, m);
		blending->setVector4("modulationColor", 1.0f, 1.0f, 1.0f, 1.0f);
		blending->setInteger("flipVertically", !onScreen);
		resetTextureBinding();
		blending->setInteger("image", (int)bindSampler(image, false));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		doRender();

		// reset texture coords
		vertexAttribBuffer[1].s = vertexAttribBuffer[3].s =
		vertexAttribBuffer[2].t = vertexAttribBuffer[3].t = 1.0f;
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(blending->getAttribLocation("inTexCoord"), 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexAttribBuffer), vertexAttribBuffer, GL_DYNAMIC_DRAW);
	}

	int getLimit(GraphicPipeline::Limit limit) const {
		switch (limit) {
		case Limit::LOCAL_GROUPS_TOTAL: return glLimits.maxTotalWorkGroupSize;
		case Limit::LOCAL_GROUPS_X: return glLimits.maxWorkGroupSize[0];
		case Limit::LOCAL_GROUPS_Y: return glLimits.maxWorkGroupSize[1];
		case Limit::LOCAL_GROUPS_Z: return glLimits.maxWorkGroupSize[2];
		case Limit::SHARED_MEM: return glLimits.maxSharedMemSize;
		}
		return 0;
	}

	int getSharedMemSize() const {
		return glLimits.maxSharedMemSize;
	}
};



GraphicPipeline::GraphicPipeline() {
	DEBUG_I("GRAPHIC PIPELINE INITIALIZATION");
	impl = new Impl(*this);
}


GraphicPipeline::~GraphicPipeline() {
	delete impl;
}


void GraphicPipeline::lock() {
	access.lock();
}


void GraphicPipeline::unlock() {
	access.unlock();
}


void GraphicPipeline::switchDisplay(void* data) {
	impl->switchDisplay(data);
}


void GraphicPipeline::swapBuffers() {
	impl->swapBuffers();
}


void GraphicPipeline::resetTextureBinding() {
	impl->resetTextureBinding();
}


unsigned int GraphicPipeline::bindSampler(GL::TextureHandler& texture, int unit) {
	return impl->bindSampler(texture, unit);
}


unsigned int GraphicPipeline::bindImage(GL::TextureHandler& texture, int unit, bool read, bool write) {
	return impl->bindImage(texture, unit, read, write);
}


void GraphicPipeline::setInterpolation(const Interpolation interpolation) {
	impl->setInterpolation(interpolation);
}


void GraphicPipeline::setOutput(AbstractBitmap& output) {
	impl->setOutput(output);
}


void GraphicPipeline::resetOutput() {
	impl->resetOutput();
}


ImageResolution GraphicPipeline::getOutputResolution() const {
	return impl->getOutputResolution();
}


void GraphicPipeline::blend(GL::TextureHandler& image, const pixfloat4& modulation, const AffineMapping& mapping) {
	impl->blend(image, modulation, mapping);
}


void GraphicPipeline::blendMasked(
	const AffineMapping& baseMapping,
	GL::TextureHandler& image,
	const AffineMapping& imageMapping,
	AbstractBitmap& mask,
	const AffineMapping& maskMapping,
	const pixfloat4& modulation,
	const pixfloat4& bgColor
) {
	impl->blendMasked(baseMapping, image, imageMapping, mask, maskMapping, modulation, bgColor);
}


void GraphicPipeline::blendShaped(
	const AffineMapping& baseMapping,
	GL::TextureHandler& image,
	const AffineMapping& imageMapping,
	const AffineMapping& maskMapping,
	const float border,
	const float slope,
	const float radius,
	const int referenceSize,
	const pixfloat4& modulation,
	const pixfloat4& bgColor
) {
	impl->blendShaped(baseMapping, image, imageMapping, maskMapping, border, slope, radius, referenceSize, modulation, bgColor);
}


void GraphicPipeline::blendCustom(GL::TextureHandler* image, const AffineMapping& mapping, GL::Program& program) {
	impl->blendCustom(image, mapping, program);
}


GL::VertexShader& GraphicPipeline::getBlendingVertexShader() const {
	return impl->getBlendingVertexShader();
}


void GraphicPipeline::paveBackground(AbstractBitmap& image) {
	impl->paveBackground(image);
}


void GraphicPipeline::fetchPixels(AbstractBitmap& bitmap) {
	impl->fetchPixels(bitmap);
}


int GraphicPipeline::getLimit(Limit limit) const {
	return impl->getLimit(limit);
}