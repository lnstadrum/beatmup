#include "texture_handler.h"
#include "../basic_types.h"
#include "../gpu/pipeline.h"
#include "../gpu/bgl.h"
#include "../debug.h"
#include <mutex>

using namespace Beatmup;
using namespace GL;

std::mutex groupCounterAccess;
unsigned int groupCounter = 0;


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
    textureHandle(0), groupIndex(0)
{}


TextureHandler::~TextureHandler() {
    if (hasValidHandle())
        BEATMUP_DEBUG_E("Destroying texture handler still having a valid handle");
}


void TextureHandler::prepare(GraphicPipeline& gpu, bool) {
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
            glhandle handle;
        public:
            Deleter(glhandle handle) : handle(handle) {}
            ~Deleter() {
                glDeleteTextures(1, &handle);
            }
        };
        bin.put(new Deleter(textureHandle));

        textureHandle = 0;
    }
}


void TextureHandler::assignToNewGroup() {
    std::lock_guard<std::mutex> lock(groupCounterAccess);
    groupIndex = ++groupCounter;
}


void TextureHandler::assignGroupFrom(const TextureHandler& another) {
    groupIndex = another.groupIndex;
}


bool TextureHandler::isOfSameGroup(const TextureHandler& another) const {
    return groupIndex == another.groupIndex;
}


bool TextureHandler::isAssignedToGroup() const {
    return groupIndex > 0;
}