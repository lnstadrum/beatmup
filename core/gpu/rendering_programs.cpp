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

#include "rendering_programs.h"
#include "gpu/pipeline.h"
#include "../gpu/bgl.h"


const char
    *Beatmup::GL::RenderingPrograms::VERTEX_COORD_ATTRIB_NAME    = "inVertex",
    *Beatmup::GL::RenderingPrograms::TEXTURE_COORD_ATTRIB_NAME   = "inTexCoord",
    *Beatmup::GL::RenderingPrograms::VERTICAL_FLIP_ID       = "flipVertically",
    *Beatmup::GL::RenderingPrograms::MODELVIEW_MATRIX_ID    = "modelview",
    *Beatmup::GL::RenderingPrograms::TEXTURE_COORDINATES_ID = "texCoord",
    *Beatmup::GL::RenderingPrograms::DECLARE_TEXTURE_COORDINATES_IN_FRAG = "varying highp vec2 texCoord;\n";


enum TextureUnits {
    IMAGE = 0,
    MASK,
    MASK_LOOKUP
};


static const char
    *VERTEX_SHADER_BLEND = BEATMUP_SHADER_CODE(
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


    *VERTEX_SHADER_BLENDMASK = BEATMUP_SHADER_CODE(
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


    *FRAGMENT_SHADER_HEADER_EXT =
        "#ifdef GL_ES\n"
        "#extension GL_OES_EGL_image_external : require\n"
        "uniform samplerExternalOES image;\n"
        "#else\n"
        "uniform sampler2D image;\n"
        "#endif\n";


using namespace Beatmup;
using namespace GL;


class RenderingPrograms::Backend {
private:
    GLuint hMaskLookups[3];			//!< texture containing mask values for 1, 2 and 4 bpp
    bool maskLookupsSet;

public:
    Backend() : maskLookupsSet(false) {}
    ~Backend() {}


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
            throw GLException("Mask bitmap pixel format is not supported");
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
};


RenderingPrograms::RenderingPrograms(GraphicPipeline* gpu):
    backend(new Backend()), currentGlProgram(nullptr), defaultVertexShader(*gpu, gpu->getGlslVersionHeader() + VERTEX_SHADER_BLEND)
{}


RenderingPrograms::~RenderingPrograms() {
    delete backend;
}


Program& RenderingPrograms::getProgram(const GraphicPipeline* gpu, Operation operation) {
    auto& map = programs;
    auto it = map.find(operation);
    if (it != map.end())
        return it->second;

    // maps shader types to internally handled shaders
    std::string fragmentCode;
    switch (operation) {
    case Operation::BLEND:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_NORMAL + FRAGMENT_SHADER_BLEND;
        break;

    case Operation::BLEND_EXT:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_EXT + FRAGMENT_SHADER_BLEND;
        break;

    case Operation::MASKED_BLEND:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_NORMAL + FRAGMENT_SHADER_BLENDMASK;
        break;

    case Operation::MASKED_BLEND_EXT:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_EXT + FRAGMENT_SHADER_BLENDMASK;
        break;

    case Operation::MASKED_8BIT_BLEND:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_NORMAL + FRAGMENT_SHADER_BLENDMASK_8BIT;
        break;

    case Operation::MASKED_8BIT_BLEND_EXT:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_EXT + FRAGMENT_SHADER_BLENDMASK_8BIT;
        break;

    case Operation::SHAPED_BLEND:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_NORMAL + FRAGMENT_SHADER_BLENDSHAPE;
        break;

    case Operation::SHAPED_BLEND_EXT:
        fragmentCode = gpu->getGlslVersionHeader() + FRAGMENT_SHADER_HEADER_EXT + FRAGMENT_SHADER_BLENDSHAPE;
        break;

    default:
        Insanity::insanity("Invalid rendering operation");
    }

    // instantiate shaders
    const bool useDefaultVertexShader = operation ==  Operation::BLEND || operation ==  Operation::BLEND_EXT;
    VertexShader* vertexShader = useDefaultVertexShader ? &defaultVertexShader : new VertexShader(*gpu, gpu->getGlslVersionHeader() + VERTEX_SHADER_BLENDMASK);
    FragmentShader fragmentShader(*gpu, fragmentCode);

    // link program
    Program& glProgram = programs.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(operation),
        std::forward_as_tuple(*gpu, *vertexShader, fragmentShader)
    ).first->second;
    if (!useDefaultVertexShader)
        delete vertexShader;
    return glProgram;
}


void RenderingPrograms::enableProgram(GraphicPipeline* gpu, Operation operation) {
    Program& glProgram = getProgram(gpu, operation);
    currentProgram = operation;
    currentGlProgram = &glProgram;
    glProgram.enable(*gpu);
    gpu->setTextureCoordinates(Rectangle::UNIT_SQUARE);
    glProgram.setInteger("image", TextureUnits::IMAGE);
    switch (operation) {
    case Operation::MASKED_BLEND:
    case Operation::MASKED_BLEND_EXT:
        glProgram.setInteger("maskLookup", TextureUnits::MASK_LOOKUP);
    case Operation::MASKED_8BIT_BLEND:
    case Operation::MASKED_8BIT_BLEND_EXT:
        glProgram.setInteger("mask", TextureUnits::MASK);
    default: break;
    }
    maskSetUp = false;
}


Program& RenderingPrograms::getCurrentProgram() {
    if (!currentGlProgram)
        RuntimeError("No current program");
    return *currentGlProgram;
}


void RenderingPrograms::bindMask(GraphicPipeline* gpu, AbstractBitmap& mask) {
    Program& program = getCurrentProgram();
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
    if (currentProgram == Operation::MASKED_BLEND || currentProgram == Operation::MASKED_BLEND_EXT)
        DebugAssertion::check(maskSetUp, "Mask was not set up in masked blending");
#endif

    getCurrentProgram().setInteger(VERTICAL_FLIP_ID, onScreen ? 0 : 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#ifdef BEATMUP_DEBUG
    GLException::check("blending");
#endif
}


void RenderingPrograms::paveBackground(GraphicPipeline* gpu, TextureHandler& content, GL::TextureHandler* output) {
    // choose program
    switch (content.getTextureFormat()) {
    case TextureHandler::TextureFormat::OES_Ext:
        enableProgram(gpu, GL::RenderingPrograms::Operation::BLEND_EXT);
        break;
    default:
        enableProgram(gpu, GL::RenderingPrograms::Operation::BLEND);
        break;
    }

    // setting texture coords, bitmap size and updating buffer data in GPU
    gpu->setTextureCoordinates(Rectangle(0, 0,
        (float)(output ? output->getWidth()  : gpu->getDisplayResolution().getWidth() ) / content.getWidth(),
        (float)(output ? output->getHeight() : gpu->getDisplayResolution().getHeight()) / content.getHeight()
    ));

    currentGlProgram->setMatrix3(MODELVIEW_MATRIX_ID, AffineMapping::IDENTITY);
    currentGlProgram->setVector4("modulationColor", 1.0f, 1.0f, 1.0f, 1.0f);
    gpu->bind(content, 0, TextureParam::REPEAT);
    blend(output == nullptr);
}


const VertexShader& RenderingPrograms::getDefaultVertexShader(const GraphicPipeline* gpu) const {
    return defaultVertexShader;
}
