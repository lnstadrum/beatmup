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

#include "texture_handler.h"
#include "../basic_types.h"
#include "../gpu/pipeline.h"
#include "../gpu/bgl.h"
#include "../debug.h"

using namespace Beatmup;
using namespace GL;

const int TextureHandler::TEXTURE_FORMAT_BYTES_PER_PIXEL[] = {
    1, 3, 4,
    4 * 1, 4 * 3, 4 * 4,
    0
};


const char* TextureHandler::textureFormatToString(const TextureFormat& format) {
    switch (format) {
    case GL::TextureHandler::TextureFormat::Rx8:
        return "Rx8";
    case GL::TextureHandler::TextureFormat::RGBx8:
        return "RGBx8";
    case GL::TextureHandler::TextureFormat::RGBAx8:
        return "RGBAx8";

    case GL::TextureHandler::TextureFormat::Rx32f:
        return "Rx32f";
    case GL::TextureHandler::TextureFormat::RGBx32f:
        return "RGBx32f";
    case GL::TextureHandler::TextureFormat::RGBAx32f:
        return "RGBAx32f";
    
    case OES_Ext:
        return "OES extension";
    }
    return "invalid format";
}

    
TextureHandler::TextureHandler() :
    textureHandle(0)
{}


TextureHandler::~TextureHandler() {
    if (hasValidHandle())
        BEATMUP_DEBUG_E("Destroying texture handler still having a valid handle");
}


void TextureHandler::prepare(GraphicPipeline& gpu) {
    if (!hasValidHandle())
        glGenTextures(1, &textureHandle);
}


const int TextureHandler::getNumberOfChannels() const {
    switch (getTextureFormat())
    {
    case GL::TextureHandler::TextureFormat::Rx8:
    case GL::TextureHandler::TextureFormat::Rx32f:
        return 1;
    case GL::TextureHandler::TextureFormat::RGBx8:
    case GL::TextureHandler::TextureFormat::RGBx32f:
        return 3;
    case GL::TextureHandler::TextureFormat::RGBAx8:
    case GL::TextureHandler::TextureFormat::RGBAx32f:
        return 4;

    case OES_Ext:
        return 3;
        //fixme
    }
    return 0;
}


void TextureHandler::invalidate(GL::RecycleBin& bin) {
    if (hasValidHandle()) {
        class Deleter : public GL::RecycleBin::Item {
            handle_t handle;
        public:
            Deleter(handle_t handle) : handle(handle) {}
            ~Deleter() {
                glDeleteTextures(1, &handle);
            }
        };
        bin.put(new Deleter(textureHandle));

        textureHandle = 0;
    }
}