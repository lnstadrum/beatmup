#include "pipeline.h"
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

    GraphicPipeline& front;

    GLuint hFrameBuffer;

    AbstractBitmap* output;

    ImageResolution
        displayResolution,					//!< width and height of a display obtained when switching
        outputResolution;					//!< actual output resolution (display or bitmap)

#ifdef BEATMUP_OPENGLVERSION_GLES
    EGLDisplay eglDisplay;
    EGLSurface
        eglSurface,				//!< currently used surface
        eglDefaultSurface;		//!< default internally managed surface
    EGLContext eglContext;
    EGLConfig eglConfig;
#elif BEATMUP_PLATFORM_WINDOWS
    HWND hwnd;
    HGLRC hglrc;
#else
    Display* xDisplay;
    Window xWindow;
    GLXContext glxContext;
    GLXPbuffer glxPbuffer;
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


public:
    Impl(GraphicPipeline& front) : front(front), output(nullptr)
    {
#ifdef BEATMUP_OPENGLVERSION_GLES
        // Step 1 - Get the default display.
        if ((eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
            throw GL::GLException("EGL: no display", eglGetError());

        // Step 2 - Initialize EGL.
        if (!eglInitialize(eglDisplay, 0, 0)) {
            auto err = eglGetError();
            if (err == EGL_NOT_INITIALIZED)
                throw GL::GLException("EGL: display not initialized", err);
            else
                throw GL::GLException("EGL: initialization failed", err);
        }

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
        if (!eglChooseConfig(eglDisplay, configAttributes, &eglConfig, 1, &numConfigs))
            throw GL::GLException("EGL: bad configuration", eglGetError());
        BEATMUP_DEBUG_I("Number of EGL configs got: %d", numConfigs);

        // Step 6 - Create a context.
        EGLint contextAttributes[] = {
            EGL_CONTEXT_CLIENT_VERSION,
#ifdef BEATMUP_OPENGLVERSION_GLES20
            2,
#elif defined BEATMUP_OPENGLVERSION_GLES31
            3,
#endif
            EGL_NONE
        };
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
        if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
            throw GL::GLException("EGL: making current", eglGetError());

#elif BEATMUP_PLATFORM_WINDOWS
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

#else
        // creating a display & a window
        xDisplay = XOpenDisplay(0);
        xWindow = XCreateSimpleWindow(xDisplay, DefaultRootWindow(xDisplay),
            0, 0,   /* x, y */
            1, 1, /* width, height */
            0, 0,     /* border_width, border */
            0);       /* background */

        // setup a bootstrap context to load glew
        static int dummy_visual_attribs[] = { GLX_RGBA, None };
        XVisualInfo* vi = glXChooseVisual(xDisplay, 0, dummy_visual_attribs);
        glxContext = glXCreateContext(xDisplay, vi, NULL, GL_TRUE);
        glXMakeCurrent(xDisplay, xWindow, glxContext);

        // power on glew
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
            throw GL::GLException((const char*)glewGetErrorString(err));

        // destroying the bootstrap context
        glXDestroyContext(xDisplay, glxContext);

        static int visual_attribs[] = {
                /*GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
                GLX_RENDER_TYPE, GLX_RGBA_BIT,*/
                GLX_DOUBLEBUFFER, true,
                None
        };
        int num_fbc = 0;
        GLXFBConfig *config = glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay),
            visual_attribs, &num_fbc);
        if (!config)
            throw GL::GLException("glXChooseFBConfig() failed");

        // create pbuffer
        static int pbuffer_attribs[] = {
                GLX_LARGEST_PBUFFER,
                None
        };
        glxPbuffer = glXCreatePbuffer(xDisplay, config[0], pbuffer_attribs);

        // create main context
        vi = glXGetVisualFromFBConfig(xDisplay, config[0]);
        glxContext = glXCreateContext(xDisplay, vi, 0, GL_TRUE);
        glXMakeCurrent(xDisplay, glxPbuffer, glxContext);

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
        BEATMUP_DEBUG_I("__________________________________________________________");
        BEATMUP_DEBUG_I("Beatmup GL startup: %s / %s", renderer, vendor);
        BEATMUP_DEBUG_I(" - Max workgroups: %d, %d, %d", glLimits.maxWorkGroupCount[0], glLimits.maxWorkGroupCount[1], glLimits.maxWorkGroupCount[2]);
        BEATMUP_DEBUG_I(" - Max local groups: %d, %d, %d / %d", glLimits.maxWorkGroupSize[0], glLimits.maxWorkGroupSize[1], glLimits.maxWorkGroupSize[2],
            glLimits.maxTotalWorkGroupSize);
        BEATMUP_DEBUG_I(" - Shared memory: %lu KB", (unsigned long)(glLimits.maxSharedMemSize / 1024));
        BEATMUP_DEBUG_I("__________________________________________________________");
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

        // setting up main controls
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
        GL::GLException::check("enabling / disabling");
    }


    ~Impl() {
        glDeleteFramebuffers(1, &hFrameBuffer);

#ifdef BEATMUP_OPENGLVERSION_GLES
        if (eglSurface != EGL_NO_SURFACE && eglSurface != eglDefaultSurface)
            eglDestroySurface(eglDisplay, eglSurface);
        eglDestroySurface(eglDisplay, eglDefaultSurface);
        eglDestroyContext(eglDisplay, eglContext);
        eglTerminate(eglDisplay);
#elif BEATMUP_PLATFORM_WINDOWS
        wglDeleteContext(hglrc);
        DestroyWindow(hwnd);
#else
        glXDestroyContext(xDisplay, glxContext);
        glXDestroyPbuffer(xDisplay, glxPbuffer);
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
        }
        else {
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


    void flush() {
        if (output) {
            output->upToDate[ProcessingTarget::GPU] = true;
            output->upToDate[ProcessingTarget::CPU] = false;
        }
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


    /**
        Binds a texture handle to an image unit
        \param[in] bitmap		The texture handler
        \param[in] imageUnit	The image unit number
        \param[in] read			If `true`, the image will be read
        \param[in] write		If `true`, the image will be modified
    */
    void bindImage(GL::TextureHandler& texture, int imageUnit, bool read, bool write) {
#ifdef BEATMUP_OPENGLVERSION_GLES20
        throw GL::Unsupported("Images binding is not supported in GL ES 2.0.");
#else
        const GL::glhandle handle = useTexture(texture);
        texture.prepare(front);
        glBindImageTexture(imageUnit,
            handle,
            0, texture.getDepth() > 1 ? GL_TRUE : GL_FALSE, 0,
            read && write ? GL_READ_WRITE : (write ? GL_WRITE_ONLY : GL_READ_ONLY),
            GL::TEXTUREHANDLER_INTERNALFORMATS[ texture.getTextureFormat() ]
        );
        GL::GLException::check("preparing image unit");
#endif
    }


    void bind(GL::TextureHandler& texture, size_t unit, const TextureParam param) {
        glActiveTexture(GL_TEXTURE0 + unit);
        switch (texture.getTextureFormat()) {
        case GL::TextureHandler::TextureFormat::Rx8:
        case GL::TextureHandler::TextureFormat::RGBx8:
        case GL::TextureHandler::TextureFormat::RGBAx8:
            useTexture(texture);
            texture.prepare(front);
            if (param & TextureParam::INTERP_LINEAR) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }
            break;

        case GL::TextureHandler::TextureFormat::Rx32f:
        case GL::TextureHandler::TextureFormat::RGBx32f:
        case GL::TextureHandler::TextureFormat::RGBAx32f:
            useTexture(texture);
            texture.prepare(front);
#ifndef BEATMUP_OPENGLVERSION_GLES
            if (param & TextureParam::INTERP_LINEAR) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }
#else
            // GLES only allows nearest interpolation for floating point texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
            break;

        case GL::TextureHandler::TextureFormat::OES_Ext:
            glBindTexture(BGL_TEXTURE_TARGET, texture.textureHandle);
            texture.prepare(front);
            glTexParameteri(BGL_TEXTURE_TARGET, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(BGL_TEXTURE_TARGET, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        }

        if (param & TextureParam::REPEAT) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        GL::GLException::check("applying texture parameter");
    }


    void bindOutput(AbstractBitmap& bitmap) {
        output = &bitmap;
        if (bitmap.isMask())
            throw GL::GLException("Mask bitmaps can not be used as output");

        // setting up a texture
        glBindFramebuffer(GL_FRAMEBUFFER, hFrameBuffer);
        GLuint handle = useTexture(bitmap);
        glBindTexture(GL_TEXTURE_2D, handle);

        // if the GPU version is outdated, feed it with blank pixels
        if (!bitmap.isUpToDate(ProcessingTarget::GPU)) {
            static const GLint formats[] = {
                0,
#ifdef BEATMUP_OPENGLVERSION_GLES20
                GL_LUMINANCE,
#else
                GL_RED,
#endif
                0, // not used
                GL_RGB, GL_RGBA
            };
            glTexImage2D(
                GL_TEXTURE_2D, 0, formats[bitmap.getNumberOfChannels()],
                bitmap.getWidth(), bitmap.getHeight(),
                0,
                formats[bitmap.getNumberOfChannels()], GL::BITMAP_PIXELTYPES[bitmap.getPixelFormat()],
                nullptr
            );
            GL::GLException::check("allocating output texture image");
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
        GLuint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (err != GL_FRAMEBUFFER_COMPLETE)
            throw GL::GLException("framebuffer incomplete", err);

        // setting up main controls
        glViewport(0, 0, bitmap.getWidth(), bitmap.getHeight());
        outputResolution = bitmap.getSize();
        glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

#ifdef BEATMUP_DEBUG
        GL::GLException::check("setting output: enabling/disabling");
#endif
    }


    void unbindOutput() {
        output = nullptr;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, displayResolution.getWidth(), displayResolution.getHeight());
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
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
        // according to GL ES 2.0 spec, only one format/type pair may be read from GPU memory directly; otherwise we need a buffer
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
            InternalBitmap buffer(bitmap.getContext(),
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


    void switchAlphaBlending(bool enable) {
        if (enable)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }
};



GraphicPipeline::GraphicPipeline(): renderingPrograms(this) {
    BEATMUP_DEBUG_I("GRAPHIC PIPELINE INITIALIZATION");
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


void GraphicPipeline::flush() {
    impl->flush();
}


void GraphicPipeline::bind(GL::TextureHandler& texture, size_t texUnit, const TextureParam param) {
    impl->bind(texture, texUnit, param);
}


void GraphicPipeline::bind(GL::TextureHandler& texture, size_t imageUnit, bool read, bool write) {
    impl->bindImage(texture, imageUnit, read, write);
}


void GraphicPipeline::bindOutput(AbstractBitmap& output) {
    impl->bindOutput(output);
}


void GraphicPipeline::unbindOutput() {
    impl->unbindOutput();
}


ImageResolution GraphicPipeline::getOutputResolution() const {
    return impl->getOutputResolution();
}


void GraphicPipeline::fetchPixels(AbstractBitmap& bitmap) {
    impl->fetchPixels(bitmap);
}


int GraphicPipeline::getLimit(Limit limit) const {
    return impl->getLimit(limit);
}


void GraphicPipeline::switchAlphaBlending(bool enable) {
    impl->switchAlphaBlending(enable);
}

const char* GraphicPipeline::getGpuVendorString() const {
    return (const char*)glGetString(GL_VENDOR);
}


const char* GraphicPipeline::getGpuRendererString() const {
    return (const char*)glGetString(GL_RENDERER);
}