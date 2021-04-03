/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pipeline.h"
#include "../bitmap/converter.h"
#include "../bitmap/internal_bitmap.h"
#include "../debug.h"
#include "program.h"

#include <algorithm>
#include <map>
#include <vector>
#include <deque>
#include <mutex>

#ifdef BEATMUP_GLES_ALLOW_DRM_FALLBACK
#include <cstdlib>
#include "drm.hpp"
#endif

#include "bgl.h"


using namespace Beatmup;

/**
    \internal
    Exception reporting issues occurred during initial GPU setup
*/
class GpuOperationError : public Beatmup::Exception {
public:
    GpuOperationError(const char* info) : Exception(info) {}
    GpuOperationError(const char* info, int code) : Exception("%s (error %x)", info, code) {}
};


/**
    \internal
    Graphic pipeline private implementation
*/
class GraphicPipeline::Impl {
private:
    /**
        Vertex attribute buffer entry: vertex coordinates (x,y) and texture coordinates (s,t)
    */
    typedef struct {
        GLfloat x, y, s, t;
    } VertexAttribBufferElement;

    GraphicPipeline& front;

    GLuint hFrameBuffer;

    VertexAttribBufferElement vertexAttribBuffer[4];
    bool isRectangularTextureCoordinates;   //!< if `true`, the texture coordinates is a rectangle
    GLuint hVertexAttribBuffer;				//!< buffer used when rendering


    ImageResolution displayResolution;    //!< width and height of a display obtained when switching

#ifdef BEATMUP_OPENGLVERSION_GLES
    EGLDisplay eglDisplay;
    EGLSurface
        eglSurface,           //!< currently used surface
        eglDefaultSurface;    //!< default internally managed surface
    EGLContext eglContext;
    EGLConfig eglConfig;
#ifdef BEATMUP_GLES_ALLOW_DRM_FALLBACK
    DRM::Device drmDevice;
    DRM::ModeConnector drmConnector;
    DRM::ModeCrtc drmCrtc;
    DRM::GBMDevice gbmDevice;
    DRM::GBMSurface gbmSurface;
#endif
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
        int maxTextureImageUnits;
        int maxFragmentUniformVectors;
        int maxWorkGroupCount[3];
        int maxWorkGroupSize[3];
        int maxTotalWorkGroupSize;
        msize maxSharedMemSize;
    } glLimits;

public:
    Impl(GraphicPipeline& front) : front(front) {

#ifdef BEATMUP_OPENGLVERSION_GLES
        // Step 1 - Get the default display.
        if ((eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
            throw GpuOperationError("EGL: no display", eglGetError());

        // Step 2 - Initialize EGL.
        if (!eglInitialize(eglDisplay, 0, 0)) {

#ifdef BEATMUP_GLES_ALLOW_DRM_FALLBACK
            BEATMUP_DEBUG_I("DRM fallback");

            // set up DRM and GBM resources
            const char* DRI_DEVICE = std::getenv("BEATMUP_DRI_DEVICE");
            static const char* DEFAULT_DRI_DEVICE = "/dev/dri/card1";
            DRM::Device device(DRI_DEVICE ? DRI_DEVICE : DEFAULT_DRI_DEVICE);
            DRM::ModeResources resources(device);
            DRM::ModeConnector connector(device, resources);
            DRM::ModeEncoder encoder(device, connector);
            DRM::ModeCrtc crtc(device, encoder, connector);
            DRM::GBMDevice gbmDevice(device, connector);
            DRM::GBMSurface gbmSurface(gbmDevice, connector);

            // get display again
            eglDisplay = eglGetDisplay(gbmDevice.getPointer());

            if (!eglDisplay) {
                throw GpuOperationError("EGL/DRM: cannot get display");
            }

            if (!eglInitialize(eglDisplay, 0, 0)) {
                throw GpuOperationError("EGL/DRM: cannot initialize display");
            }

            // keep DRM and GBM persistent resources
            this->drmDevice = std::move(device);
            this->drmConnector = std::move(connector);
            this->drmCrtc = std::move(crtc);
            this->gbmDevice = std::move(gbmDevice);
            this->gbmSurface = std::move(gbmSurface);
#else
            auto err = eglGetError();
            if (err == EGL_NOT_INITIALIZED)
                throw GpuOperationError("EGL: display not initialized", err);
            else
                throw GpuOperationError("EGL: initialization failed", err);
#endif
        }

        // Step 3 - Make OpenGL ES the current API.
        eglBindAPI(EGL_OPENGL_ES_API);

        // Step 4 - Specify the required configuration attributes.
        static const int CONFIG_ATTRIBUTES_LEN = 5;
        EGLint configAttributes[CONFIG_ATTRIBUTES_LEN] = {
#ifdef BEATMUP_OPENGLVERSION_GLES20
            EGL_RENDERABLE_TYPE,		EGL_OPENGL_ES2_BIT,
#elif defined BEATMUP_OPENGLVERSION_GLES31
            EGL_RENDERABLE_TYPE,		EGL_OPENGL_ES3_BIT_KHR,
#endif
            EGL_SURFACE_TYPE,			EGL_PBUFFER_BIT,
            EGL_NONE
        };

#ifdef BEATMUP_GLES_ALLOW_DRM_FALLBACK
        if (drmDevice) {
            // using DRM: shrinking config attributes at EGL_SURFACE_TYPE
            configAttributes[CONFIG_ATTRIBUTES_LEN - 3] = EGL_NONE;
        }
#endif

        // get number of configs
        int totalConfigCount;
        eglGetConfigs(eglDisplay, nullptr, 0, &totalConfigCount);
        std::vector<EGLConfig> configs;
        configs.resize(totalConfigCount);

        // get configs themselves
        int numConfigs;
        if (!eglChooseConfig(eglDisplay, configAttributes, configs.data(), totalConfigCount, &numConfigs))
            throw GpuOperationError("EGL: bad configuration", eglGetError());
        BEATMUP_DEBUG_I("Number of EGL configs got: %d", numConfigs);

#ifdef BEATMUP_GLES_ALLOW_DRM_FALLBACK
        if (drmDevice) {
            // find one matching the GBM format
            bool found = false;
            for (int i = 0; i < totalConfigCount && !found; ++i) {
                EGLint id;
                if (!eglGetConfigAttrib(eglDisplay, configs[i], EGL_NATIVE_VISUAL_ID, &id))
                    continue;
                if (id == DRM::GBMSurface::FORMAT) {
                    eglConfig = configs[i];
                    found = true;
                }
            }
            if (!found)
                throw GpuOperationError("EGL/DRM: no config matching the required surface format");
        }
        else
#endif
        {
            eglConfig = configs[0];
        }


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
            throw GpuOperationError("EGL: context initialization failed", eglGetError());

        eglDefaultSurface = eglSurface = EGL_NO_SURFACE;

        // Step 5 - Create a surface to draw to.
#ifdef BEATMUP_GLES_ALLOW_DRM_FALLBACK
        if (drmDevice) {
            eglDefaultSurface = eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, gbmSurface.getPointer(), nullptr);
        }
        else
#endif
        {
            EGLint surfaceAttributes[] = {
                EGL_WIDTH, 2,		// eglMakeCurrent fails sometimes with zero sizes
                EGL_HEIGHT, 2,
                EGL_NONE
            };

            eglDefaultSurface = eglSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, surfaceAttributes);
        }

        if (eglSurface == EGL_NO_SURFACE)
            throw GpuOperationError("EGL: window surface creation failed when init", eglGetError());

        // Step 7 - Bind the context to the current thread
        if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
            throw GpuOperationError("EGL: making current", eglGetError());

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
        hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
#ifdef UNICODE
            L"STATIC", L"glctx",
#else
            "STATIC", "glctx",
#endif
            WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 1, 1, 0, 0, GetModuleHandle(NULL), 0);
        if (!hwnd)
            throw GpuOperationError("Unable to initialize GL context");

        ShowWindow(hwnd, SW_HIDE);
        HDC hdc = GetDC(hwnd);
        int pixelFormat = ChoosePixelFormat(hdc, &pfd);
        SetPixelFormat(hdc, pixelFormat, &pfd);
        hglrc = wglCreateContext(hdc);
        wglMakeCurrent(hdc, hglrc);
        if (!wglGetCurrentContext())
            throw GpuOperationError("Unable to initialize GL context");

        // init glew
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
            throw GpuOperationError((const char*)glewGetErrorString(err));

#else
        // creating a display & a window
        xDisplay = XOpenDisplay(0);
        if (xDisplay == nullptr)
            throw GpuOperationError("Cannot open a display connection to X11 server");
        xWindow = XCreateSimpleWindow(xDisplay, DefaultRootWindow(xDisplay),
            0, 0,     /* x, y */
            1, 1,     /* width, height */
            0, 0,     /* border_width, border */
            0);       /* background */

        // setup a bootstrap context to load glew
        static int dummy_visual_attribs[] = { GLX_RGBA, None };
        XVisualInfo* vi = glXChooseVisual(xDisplay, 0, dummy_visual_attribs);
        glxContext = glXCreateContext(xDisplay, vi, nullptr, GL_TRUE);
        glXMakeCurrent(xDisplay, xWindow, glxContext);

        // power on glew
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
            throw GpuOperationError((const char*)glewGetErrorString(err));

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
            throw GpuOperationError("Choosing framebuffer configuration failed");

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
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &glLimits.maxTextureImageUnits);
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &glLimits.maxFragmentUniformVectors);
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
        BEATMUP_DEBUG_I(" - Max workgroups: %d, %d, %d",
            glLimits.maxWorkGroupCount[0], glLimits.maxWorkGroupCount[1], glLimits.maxWorkGroupCount[2]);
        BEATMUP_DEBUG_I(" - Max local groups: %d, %d, %d / %d",
            glLimits.maxWorkGroupSize[0], glLimits.maxWorkGroupSize[1], glLimits.maxWorkGroupSize[2], glLimits.maxTotalWorkGroupSize);
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

        // init attribute buffers
        glGenBuffers(1, &hVertexAttribBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, hVertexAttribBuffer);
        vertexAttribBuffer[0] = { 0.0f, 0.0f, 0.0f, 0.0f };
        vertexAttribBuffer[1] = { 1.0f, 0.0f, 1.0f, 0.0f };
        vertexAttribBuffer[2] = { 0.0f, 1.0f, 0.0f, 1.0f };
        vertexAttribBuffer[3] = { 1.0f, 1.0f, 1.0f, 1.0f };
        isRectangularTextureCoordinates = true;

        glEnableVertexAttribArray(GraphicPipeline::ATTRIB_VERTEX_COORD);
        glEnableVertexAttribArray(GraphicPipeline::ATTRIB_TEXTURE_COORD);
        glVertexAttribPointer(GraphicPipeline::ATTRIB_VERTEX_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), nullptr);
        glVertexAttribPointer(GraphicPipeline::ATTRIB_TEXTURE_COORD, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAttribBufferElement), (void*)(2 * sizeof(GLfloat)));
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexAttribBuffer), vertexAttribBuffer, GL_STATIC_DRAW);

        // setting up main controls
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
        switchMode(GraphicPipeline::Mode::RENDERING);
        GL::GLException::check("initial GPU setup");
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
        XCloseDisplay(xDisplay);
#endif
    }


    /**
        Platform-dependent display switching routine
    */
    void switchDisplay(void* data) {
#ifdef BEATMUP_PLATFORM_WINDOWS
        throw GL::Unsupported("switchDisplay is not implemented on Windows");
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
                throw GpuOperationError("EGL: window surface creation failed when switching display", eglGetError());

            // bind the context to the current thread
            if (! eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
                throw GpuOperationError("EGL: switching display", eglGetError());

            // setting viewport
            displayResolution.set(ANativeWindow_getWidth((ANativeWindow*)data), ANativeWindow_getHeight((ANativeWindow*)data));
            glViewport(0, 0, displayResolution.getWidth(), displayResolution.getHeight());
        }
        else {
            eglSurface = eglDefaultSurface;
            if (! eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
                throw GpuOperationError("EGL: making current", eglGetError());
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
        throw GL::Unsupported("swapBuffers is not implemented on Windows");
#elif BEATMUP_OPENGLVERSION_GLES
        if (! eglSwapBuffers(eglDisplay, eglSurface))
            throw GpuOperationError("EGL: swapping buffers", eglGetError());
#endif
    }


    /**
        Binds a texture handle to an image unit
        \param[in] texture      The texture handler
        \param[in] imageUnit    The image unit number
        \param[in] read         If `true`, the image will be read
        \param[in] write        If `true`, the image will be modified
    */
    void bindImage(GL::TextureHandler& texture, int imageUnit, bool read, bool write) {
#ifdef BEATMUP_OPENGLVERSION_GLES20
        throw GL::Unsupported("Images binding is not supported in GL ES 2.0.");
#else

        texture.prepare(front);

        // if the following is not set, black images are out when writing with imageStore()
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // binding, actually
        glBindImageTexture(imageUnit,
            texture.textureHandle,
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
            texture.prepare(front);
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


    void bindOutput(AbstractBitmap& bitmap, const IntRectangle& viewport) {
        if (bitmap.isMask())
            throw GL::GLException("Mask bitmaps can not be used as output");
        bitmap.prepare(front);
        bindOutput(bitmap.textureHandle);
        glViewport(viewport.getX1(), viewport.getY1(), viewport.width(), viewport.height());
        glClear(GL_COLOR_BUFFER_BIT);
    }


    void bindOutput(GL::TextureHandler& textureHandler) {
        textureHandler.prepare(front);
        bindOutput(textureHandler.textureHandle);
        glViewport(0, 0, textureHandler.getWidth(), textureHandler.getHeight());
    }


    void bindOutput(GL::handle_t texture) {
        // setting up a texture
        glBindFramebuffer(GL_FRAMEBUFFER, hFrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
#ifdef BEATMUP_DEBUG
        GLuint err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (err != GL_FRAMEBUFFER_COMPLETE)
            throw GL::GLException("framebuffer incomplete", err);
#endif
    }


    void unbindOutput() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, displayResolution.getWidth(), displayResolution.getHeight());
        glClear(GL_COLOR_BUFFER_BIT);
    }


    const ImageResolution& getDisplayResolution() const {
        return displayResolution;
    }

    /**
        Transfers texture data from GPU to CPU. The bitmap is assumed locked.
    */
    void pullPixels(AbstractBitmap& bitmap) {
#ifdef BEATMUP_DEBUG
        DebugAssertion::check(bitmap.getData(0, 0) != nullptr, "Cannot pull pixels: the bitmap is not locked");
#endif

#ifdef BEATMUP_OPENGLVERSION_GLES20
        if (bitmap.isFloat())
            throw GL::Unsupported("Floating point valued bitmaps are not updatable from GPU memory");
                // 'cause GLES does not support such data to be transferred from GPU to CPU memory
#endif

        glBindFramebuffer(GL_FRAMEBUFFER, hFrameBuffer);
        bitmap.prepare(front);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bitmap.textureHandle, 0);

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

            {
                AbstractBitmap::WriteLock<ProcessingTarget::CPU> lock(buffer);
                glReadPixels(0, 0, buffer.getWidth(), buffer.getHeight(),
                    GL::BITMAP_PIXELFORMATS[buffer.getPixelFormat()],
                    GL::BITMAP_PIXELTYPES[buffer.getPixelFormat()],
                    buffer.getData(0, 0)
                );
                GL::GLException::check("reading pixel data from GPU memory");
            }

            // converting to the required format
            FormatConverter::convert(buffer, bitmap);
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


    /**
        Transfers texture data from CPU to GPU. The bitmap is assumed locked.
    */
    void pushPixels(AbstractBitmap& bitmap) {
        bitmap.prepare(front);

        // pushing data if any
        if (bitmap.getTextureFormat() != GL::TextureHandler::TextureFormat::OES_Ext) {
#ifdef BEATMUP_DEBUG
            DebugAssertion::check(bitmap.getData(0, 0) != nullptr, "Cannot push pixels: the bitmap is not locked");
#endif

            // setup alignment
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            if (bitmap.isMask()) {
                // masks are stored as horizontally-stretched bitmaps
                const int textureWidth = bitmap.getWidth() / (8 / bitmap.getBitsPerPixel());

#ifdef BEATMUP_OPENGLVERSION_GLES20
                glTexImage2D(GL_TEXTURE_2D,
                    0, GL_ALPHA,
                    textureWidth, bitmap.getHeight(),
                    0, GL_ALPHA, GL_UNSIGNED_BYTE,
                    bitmap.getData(0, 0));
#else
                glTexSubImage2D(GL_TEXTURE_2D,
                    0, 0, 0, textureWidth, bitmap.getHeight(),
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    bitmap.getData(0, 0));
#endif
                GL::GLException::check("sending texture data (mask)");
            }

            else {
#ifdef BEATMUP_OPENGLVERSION_GLES20
                glTexImage2D(GL_TEXTURE_2D,
                    0,
                    GL::BITMAP_INTERNALFORMATS[bitmap.getPixelFormat()],
                    bitmap.getWidth(), bitmap.getHeight(),
                    0,
                    GL::BITMAP_PIXELFORMATS[bitmap.getPixelFormat()],
                    GL::BITMAP_PIXELTYPES[bitmap.getPixelFormat()],
                    bitmap.getData(0, 0));
#else
                glTexSubImage2D(GL_TEXTURE_2D,
                    0, 0, 0, bitmap.getWidth(), bitmap.getHeight(),
                    GL::BITMAP_PIXELFORMATS[bitmap.getPixelFormat()],
                    GL::BITMAP_PIXELTYPES[bitmap.getPixelFormat()],
                    bitmap.getData(0, 0));
#endif
                GL::GLException::check("sending texture data");
            }
        }

        bitmap.upToDate[ProcessingTarget::GPU] = true;
    }


    int getLimit(GraphicPipeline::Limit limit) const {
        switch (limit) {
        case Limit::TEXTURE_IMAGE_UNITS: return glLimits.maxTextureImageUnits;
        case Limit::FRAGMENT_UNIFORM_VECTORS: return glLimits.maxFragmentUniformVectors;
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


    void switchMode(GraphicPipeline::Mode mode) {
        if (mode == GraphicPipeline::Mode::RENDERING) {
            glEnable(GL_BLEND);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        } else if (mode == GraphicPipeline::Mode::INFERENCE) {
            glDisable(GL_BLEND);
            glClearColor(0, 0, 0, 0);
        }
    }


    void setTextureCoordinates(const Rectangle& coords) {
        if (vertexAttribBuffer[0].s != coords.getX1() ||
            vertexAttribBuffer[1].s != coords.getX2() ||
            vertexAttribBuffer[0].t != coords.getY1() ||
            vertexAttribBuffer[2].t != coords.getY2() ||
            !isRectangularTextureCoordinates)
        {
            vertexAttribBuffer[0].s = vertexAttribBuffer[2].s = coords.getX1();
            vertexAttribBuffer[1].s = vertexAttribBuffer[3].s = coords.getX2();
            vertexAttribBuffer[0].t = vertexAttribBuffer[1].t = coords.getY1();
            vertexAttribBuffer[2].t = vertexAttribBuffer[3].t = coords.getY2();
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexAttribBuffer), vertexAttribBuffer);
            isRectangularTextureCoordinates = true;
        }
#ifdef BEATMUP_DEBUG
        GL::GLException::check("vertex attributes buffer setup");
#endif
    }

    void setTextureCoordinates(const Point& leftTop, const Point& rightTop, const Point& leftBottom, const Point& rightBottom) {
        vertexAttribBuffer[0].s = leftTop.x;
        vertexAttribBuffer[0].t = leftTop.y;
        vertexAttribBuffer[1].s = rightTop.x;
        vertexAttribBuffer[1].t = rightTop.y;
        vertexAttribBuffer[2].s = leftBottom.x;
        vertexAttribBuffer[2].t = leftBottom.y;
        vertexAttribBuffer[3].s = rightBottom.x;
        vertexAttribBuffer[3].t = rightBottom.y;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertexAttribBuffer), vertexAttribBuffer);
        isRectangularTextureCoordinates = false;
#ifdef BEATMUP_DEBUG
        GL::GLException::check("vertex attributes buffer setup");
#endif
    }
};



GraphicPipeline::GraphicPipeline() {
    BEATMUP_DEBUG_I("GRAPHIC PIPELINE INITIALIZATION");
    impl = new Impl(*this);
    renderingPrograms = new GL::RenderingPrograms(this);
}


GraphicPipeline::~GraphicPipeline() {
    delete renderingPrograms;
    delete impl;
}


void GraphicPipeline::switchDisplay(void* data) {
    impl->switchDisplay(data);
}


void GraphicPipeline::swapBuffers() {
    impl->swapBuffers();
}


void GraphicPipeline::bind(GL::TextureHandler& texture, size_t texUnit, const TextureParam param) {
    impl->bind(texture, texUnit, param);
}


void GraphicPipeline::bind(GL::TextureHandler& texture, size_t imageUnit, bool read, bool write) {
    impl->bindImage(texture, imageUnit, read, write);
}


void GraphicPipeline::bindOutput(AbstractBitmap& bitmap) {
    impl->bindOutput(bitmap, bitmap.getSize().halfOpenedRectangle());
}


void GraphicPipeline::bindOutput(AbstractBitmap& bitmap, const IntRectangle& viewport) {
    impl->bindOutput(bitmap, viewport);
}


void GraphicPipeline::bindOutput(GL::TextureHandler& textureHandler) {
    impl->bindOutput(textureHandler);
}


void GraphicPipeline::bindOutput(GL::handle_t texture) {
    impl->bindOutput(texture);
}


void GraphicPipeline::unbindOutput() {
    impl->unbindOutput();
}


const ImageResolution& GraphicPipeline::getDisplayResolution() const {
    return impl->getDisplayResolution();
}


void GraphicPipeline::pullPixels(AbstractBitmap& bitmap) {
    impl->pullPixels(bitmap);
}


void GraphicPipeline::pushPixels(AbstractBitmap& bitmap) {
    impl->pushPixels(bitmap);
}


void GraphicPipeline::flush() {
    glFinish();
}


int GraphicPipeline::getLimit(Limit limit) const {
    return impl->getLimit(limit);
}


void GraphicPipeline::switchMode(Mode mode) {
    impl->switchMode(mode);
}


const char* GraphicPipeline::getGpuVendorString() const {
    return (const char*)glGetString(GL_VENDOR);
}


const char* GraphicPipeline::getGpuRendererString() const {
    return (const char*)glGetString(GL_RENDERER);
}


void GraphicPipeline::setTextureCoordinates(const Beatmup::Rectangle& coords) {
    impl->setTextureCoordinates(coords);
}


void GraphicPipeline::setTextureCoordinates(const Point& leftTop, const Point& rightTop, const Point& leftBottom, const Point& rightBottom) {
    impl->setTextureCoordinates(leftTop, rightTop, leftBottom, rightBottom);
}


Beatmup::Rectangle GraphicPipeline::getTextureCoordinates(const Beatmup::Rectangle& area, const IntPoint& size, const IntPoint& sampling) {
    /* Common OpenGL texture sampling model, likely:
        . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .    <- texture pixels (`size` in number).
           |  *     *     *     *     *     *     *     *     *  |               <- sampling point locations (`area`).
           |                                                     |                  There are exactly `sampling` points.
           |_____________________________________________________|               <- texture coordinates to give to
                                                                                    the shaders to get the texture
                                                                                    sampled in the starred locations.
    */

    // compute offsets of texture coordinates with respect to the sampling positions
    const float
        dx = sampling.x > 1 ?  0.5f * area.width()  / (sampling.x - 1)  : 0.0f,
        dy = sampling.y > 1 ?  0.5f * area.height() / (sampling.y - 1)  : 0.0f;

    // compute texture coordinates: x + 0.5 is the target sampling position, dx is the offset
    return Beatmup::Rectangle(
        (area.a.x + 0.5f - dx) / size.x,
        (area.a.y + 0.5f - dy) / size.y,
        (area.b.x + 0.5f + dx) / size.x,
        (area.b.y + 0.5f + dy) / size.y
    );
}
