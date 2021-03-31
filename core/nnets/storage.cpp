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

#include "storage.h"
#include "../gpu/bgl.h"
#include "../gpu/texture_handler.h"
#include "../utils/string_builder.h"
#include "../utils/bitset.h"
#include "../exception.h"
#include "../platform.h"
#include <cstring>
#include <map>


using namespace Beatmup;
using namespace NNets;


const Size Size::EMPTY(0, 0, 0);
const Size Size::ONES (1, 1, 1);


static inline void bind(int unit, GL::handle_t texture) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef BEATMUP_DEBUG
    GL::GLException::check("binding storage texture");
#endif
}




Size::Size(int width, int height, int depth) {
    dim[0] = width;
    dim[1] = height;
    dim[2] = depth;
}


Size Size::transform(Size kernel, Size stride, Padding padding, int depth) const {
    Size result(dim[0], dim[1], depth == 0 ? this->dim[2] : depth);
    if (padding == Padding::SAME) {
        result.dim[0] = ceili(dim[0], stride[0]);
        result.dim[1] = ceili(dim[1], stride[1]);
    }
    else {
        result.dim[0] = ceili(dim[0] - kernel[0] + 1, stride[0]);
        result.dim[1] = ceili(dim[1] - kernel[1] + 1, stride[1]);
    }
    return result;
}


Size Size::getOrigin(Size kernel, Size stride, Padding padding) const {
    if (padding == Padding::SAME) {
        return Size(
            kernel[0] / 2 - (kernel[0] - ((dim[0] - 1) % stride[0]) - 1) / 2,
            kernel[1] / 2 - (kernel[1] - ((dim[1] - 1) % stride[1]) - 1) / 2,
            kernel[2] / 2 - (kernel[2] - ((dim[2] - 1) % stride[2]) - 1) / 2
        );
    }
    else {
        return Size(kernel[0] / 2, kernel[1] / 2, kernel[2] / 2);
    }
}


void Storage::push(GraphicPipeline& gpu, const void* data) {
    const uint8_t* ptr = (const uint8_t*)data;
    const size_t textureSizeBytes =  getTextureWidth() * getTextureHeight() * 4;

    const GL::TextureHandler::TextureFormat format = GL::TextureHandler::TextureFormat::RGBAx8;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (int i = 0; i < getNumberOfTextures(); ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i].handle);
#ifdef BEATMUP_OPENGLVERSION_GLES20
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL::BITMAP_INTERNALFORMATS[format],
            getTextureWidth(), getTextureHeight(),
            0,
            GL_RGBA,
            GL::BITMAP_PIXELTYPES[format],
            data ? ptr : nullptr);
        if (data)
             ptr += textureSizeBytes;
#else
        glTexStorage2D(GL_TEXTURE_2D, 1, GL::BITMAP_INTERNALFORMATS[format], getTextureWidth(), getTextureHeight());
        if (data) {
            glTexSubImage2D(GL_TEXTURE_2D,
                0, 0, 0, getTextureWidth(), getTextureHeight(),
                GL_RGBA,
                GL::BITMAP_PIXELTYPES[format],
                ptr
            );
            ptr += textureSizeBytes;
        }
#endif

        // set dirty flag if no data (the texture needs to be cleared before being used)
        textures[i].dirty = (data == nullptr);
        GL::GLException::check("allocating storage");
    }

    upToDate[ProcessingTarget::GPU] = true;
}


Storage::Storage(Context& ctx, GraphicPipeline& gpu, const Size size, const int pad, const int reservedChannels):
    context(ctx),
    textures(nullptr), size(size), pad(pad),
    upToDate{false, false}
{
    const int depth = size.getDepth() + reservedChannels;
    checkChannelNumber(depth);

    // decide on packing
    const int maxChannels = 4 * gpu.getLimit(GraphicPipeline::Limit::TEXTURE_IMAGE_UNITS);
    if (depth <= maxChannels) {
        packX = packY = 1;
    }
    else {
        const int channelsPerTexture = ceili(depth, maxChannels);
        for (int i = 1; i*i <= channelsPerTexture; ++i)
            if (channelsPerTexture % i == 0) {
                packX = i;
                packY = channelsPerTexture / i;
            }
    }
}


Storage::Storage(Context& ctx, GraphicPipeline& gpu, const Size size):
    context(ctx),
    textures(nullptr), size(size), pad(0),
    upToDate{false, false}
{
    checkChannelNumber(size.getDepth());
    packX = 1;
    packY = size.getDepth() / 4;
}


Storage::~Storage() {
    free();
}


void Storage::allocate(GraphicPipeline& gpu) {
    if (textures)
        return;

    // setting up textures
    const int count = getNumberOfTextures();
    textures = new Texture[count];
    for (int i = 0; i < count; ++i)
        glGenTextures(1, &textures[i].handle);

    push(gpu, nullptr);
}


void Storage::allocate() {
    if (memory)
        return;

    memory = AlignedMemory(getMemorySize());
    upToDate[ProcessingTarget::CPU] = true;
}


void Storage::free(GraphicPipeline& gpu) {
    // feeing GPU storage
    if (textures != nullptr) {
        for (int i = 0; i < getNumberOfTextures(); ++i)
            glGenTextures(1, &textures[i].handle);
        delete[] textures;
        textures = nullptr;
    }

    // freeing CPU storage
    memory.free();
}


void Storage::free() {
    class Deleter : public GL::RecycleBin::Item {
        GL::handle_t* handles;
        int count;
    public:
        Deleter(const Texture* textures, int count) : handles(new GL::handle_t[count]), count(count) {
            for (int i = 0; i < count; ++i)
                handles[i] = textures[i].handle;
        }
        ~Deleter() {
            glDeleteTextures(count, handles);
            delete[] handles;
        }
    };

    // freeing GPU storage
    if (textures != nullptr) {
        context.getGpuRecycleBin()->put(new Deleter(textures, getNumberOfTextures()));
        textures = nullptr;
    }

    // freeing CPU storage
    memory.free();

    upToDate[ProcessingTarget::CPU] = false;
    upToDate[ProcessingTarget::GPU] = false;
}


void Storage::pull(GraphicPipeline& gpu) {
    if (!upToDate[ProcessingTarget::GPU])
        throw InconsistentStorageState("No data to pull in the storage");

    const int textureSizeBytes = getTextureWidth() * getTextureHeight() * 4;

    // allocate / acquire CPU storage
    if (!memory)
        memory = AlignedMemory(textureSizeBytes * getNumberOfTextures());
    uint8_t* ptr = memory.ptr<uint8_t>(0);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    for (int i = 0; i < getNumberOfTextures(); ++i) {
        gpu.bindOutput(textures[i].handle);
        glReadPixels(0, 0, getTextureWidth(), getTextureHeight(),
             GL_RGBA, GL_UNSIGNED_BYTE,
             ptr
        );
        ptr += textureSizeBytes;
        GL::GLException::check("pulling storage, texture " + std::to_string(i) + " of " + std::to_string(getNumberOfTextures()));
    }

    upToDate[ProcessingTarget::CPU] = true;
}


void Storage::push(GraphicPipeline& gpu) {
    if (!upToDate[ProcessingTarget::CPU])
        throw InconsistentStorageState("No data to push in the storage");

    // allocate on GPU if not yet
    if (!textures) {
        const int count = getNumberOfTextures();
        textures = new Texture[count];
        for (int i = 0; i < count; ++i) {
            glGenTextures(1, &textures[i].handle);
            textures[i].dirty = true;
        }
    }

    push(gpu, memory());
}


void Storage::push(GraphicPipeline& gpu, const float* hwcData, const size_t numSamples) {
    RuntimeError::check(numSamples == getSize().volume(), "Data size does not match storage capacity");

    // allocate the CPU storage if not yet
    if (!memory) {
        allocate();
        memset(memory(), 0, getMemorySize());
    }

#ifdef BEATMUP_DEBUG
    // check the input data range
    for (int i = 0; i < numSamples; ++i)
        OutOfRange::check<float>(hwcData[i], 0, 1, "Data contains a sample falling out of 0..1 range: %0.8f");
#endif

    // push
    const int width = size.getWidth(), height = size.getHeight(), depth = size.getDepth();
    const int paddedWidth = width + 2 * pad;
    const int paddedHeight = height + 2 * pad;
    for (int c = 0; c < depth; c+=4)
        for (int y = 0, i = 0; y < height; ++y) {
            color4i* dst = memory.ptr<color4i>(paddedWidth * paddedHeight * c / 4 + paddedWidth * (y + pad) + pad);
            for (int x = 0; x < width; ++x, ++dst, ++i) {
                dst->r = (uint8_t)(hwcData[i * depth + c + 0] * 255);
                dst->g = (uint8_t)(hwcData[i * depth + c + 1] * 255);
                dst->b = (uint8_t)(hwcData[i * depth + c + 2] * 255);
                dst->a = (uint8_t)(hwcData[i * depth + c + 3] * 255);
            }
        }

    upToDate[ProcessingTarget::CPU] = true;
    push(gpu);
}


InternalBitmap* Storage::getImage(Context& ctx, GraphicPipeline& gpu, int channel) const {
    RuntimeError::check(isAllocated(), "The storage contains no data because it is not yet allocated.");
    RuntimeError::check(0 <= channel && channel < size.getDepth(), "Channel number is out of range.");

    static const bool crop = true;    // crop the specific channel instead of saving the whole texture

    // prepare code
    String code;
    code.printf("%s" BEATMUP_SHADER_CODE(
        uniform sampler2D image;
        varying highp vec2 texCoord;
        void main() {
            lowp float v = texture2D(image, texCoord)[%d];
            gl_FragColor = vec4(v, v, v, 1.0);
        }
    ), gpu.getGlslVersionHeader().c_str(), channel % 4);

    // init bitmap, set as output
    InternalBitmap* bitmap = new InternalBitmap(ctx, PixelFormat::QuadByte,
        crop ? size[0] : getTextureWidth(),
        crop ? size[1] : getTextureHeight(),
        true);

    // setup shaders
    GL::FragmentShader shader(gpu, code);
    GL::RenderingProgram program(gpu, shader);

    // init binder
    const IntPoint origin = getChannelOrigin(channel / 4 * 4);
    Binder binder(gpu);
    binder.begin(program, *bitmap);
    gpu.setTextureCoordinates(
        crop ?
            IntRectangle(origin.x, origin.y, origin.x + size[0] - 1, origin.y + size[1] - 1) :
            IntRectangle(0, 0, getTextureWidth() - 1, getTextureHeight() - 1),
        IntPoint(getTextureWidth(), getTextureHeight()),
        bitmap->getSize()
    );

    // bind input
    bind(0, textures[getChannelTextureNumber(channel / 4 * 4)].handle);

    // go
    {
        AbstractBitmap::WriteLock<ProcessingTarget::GPU> lock(*bitmap);
        program.blend();
    }
    gpu.pullPixels(*bitmap);

    return bitmap;
}


int Storage::getNumberOfTextures() const {
    return ceili(size.getDepth(), 4 * packX * packY);
}


int Storage::getChannelTextureNumber(int channel) const {
#ifdef BEATMUP_DEBUG
    checkChannelNumber(channel);
    OutOfRange::check(channel, 0, size[2] - 1, "Channel out of range: %d");
#endif
    return channel / 4 / (packX * packY);
}


IntPoint Storage::getChannelOrigin(int channel) const {
#ifdef BEATMUP_DEBUG
    checkChannelNumber(channel);
#endif
    return IntPoint(
        pad + (channel / 4 % packX) * (pad + size[0]),
        pad + ((channel / 4 / packX) % packY) * (pad + size[1])
    );
}


int Storage::getTextureWidth() const {
    return (size.getWidth() + pad) * packX + pad;
}


int Storage::getTextureHeight() const {
    return (size.getHeight() + pad) * packY + pad;
}




Storage::View::View(const View& another) {
    channels = another.channels;
    textures = another.textures;
    storage = another.storage;
}


Storage::View::View(View&& another) {
    channels.swap(another.channels);
    textures.swap(another.textures);
    storage = another.storage;
    another.storage = nullptr;
}


Storage::View& Storage::View::operator=(View&& another) {
    channels.swap(another.channels);
    textures.swap(another.textures);
    storage = another.storage;
    another.storage = nullptr;
    return *this;
}


Storage::View::View(Storage& storage):
    channels(storage.getSize()[2] / 4),
    textures(storage.getNumberOfTextures()),
    storage(&storage)
{
    const int num = storage.getSize()[2] / 4;
    for (int i = 0; i < num; ++i) {
        channels[i].textureIdx = storage.getChannelTextureNumber(4 * i);
        channels[i].channelIdx = 4 * i;
    }
    for (int i = 0; i < storage.getNumberOfTextures(); ++i)
        textures[i] = i;
}


Storage::View::View(View&& view, const int firstChannel, const int numChannels):
    storage(view.storage)
{
#ifdef BEATMUP_DEBUG
    Storage::checkChannelNumber(firstChannel);
    Storage::checkChannelNumber(firstChannel + numChannels);
    OutOfRange::check(firstChannel + numChannels, 0, view.getDepth(), "Number of channels out of range: %d");
#endif
    Bitset usedTextures(storage->getNumberOfTextures(), false);
    for (int i = firstChannel; i < firstChannel + numChannels; i += 4)
        usedTextures.set(view.textures[view.getChannelTextureNumber(i)]);

    textures.reserve(usedTextures.count());
    std::map<int, int> textureMap;
    for (int i = 0; i < storage->getNumberOfTextures(); ++i)
        if (usedTextures[i]) {
            textureMap[i] = textures.size();
            textures.emplace_back(i);
        }

    channels.reserve(numChannels / 4);
    for (int i = firstChannel; i < firstChannel + numChannels; i += 4) {
        const auto entry = view.channels[i / 4];
        channels.push_back(Channel{ entry.channelIdx, textureMap[entry.textureIdx] });
    }
}


Storage::View::View(Storage& storage, const int shuffleStep):
    channels(storage.getSize()[2] / 4),
    textures(storage.getNumberOfTextures()),
    storage(&storage)
{
    const int num = storage.getSize()[2] / 4;
#ifdef BEATMUP_DEBUG
    OutOfRange::checkMin(shuffleStep, 1, "Shuffling step must be positive, but %d got");
#endif
    RuntimeError::check(num % shuffleStep == 0, "Shuffling step *4 must be a divider of the storage depth");

    for (int i = 0; i < storage.getNumberOfTextures(); ++i)
        textures[i] = i;

    for (int i = 0; i < num; ++i) {
        const int shuffled = 4 * ((shuffleStep * i) % num + (shuffleStep * i) / num);
        channels[i].textureIdx = storage.getChannelTextureNumber(shuffled);
        channels[i].channelIdx = shuffled;
    }
}


InternalBitmap* Storage::View::getImage(Context& ctx, GraphicPipeline& gpu, int channel) const {
#ifdef BEATMUP_DEBUG
    OutOfRange::check(channel, 0, getDepth() - 1, "Channel index out of range: %d");
#endif
    const int storageChannel = channels[channel / 4].channelIdx + (channel % 4);
    return storage->getImage(ctx, gpu, storageChannel);
}


int Storage::View::getChannelTextureNumber(int channel) const {
#ifdef BEATMUP_DEBUG
    Storage::checkChannelNumber(channel);
    OutOfRange::check(channel, 0, getDepth() - 1, "Channel index out of range: %d");
#endif
    return channels[channel / 4].textureIdx;
}


IntPoint Storage::View::getChannelOrigin(int channel) const {
#ifdef BEATMUP_DEBUG
    Storage::checkChannelNumber(channel);
    OutOfRange::check(channel, 0, getDepth() - 1, "Channel index out of range: %d");
#endif
    const Channel& ch = channels[channel / 4];
    return storage->getChannelOrigin(ch.channelIdx);
}


Storage::TextureHandler::TextureHandler(const View& view, int channel):
    width(view.getStorage().getTextureWidth()), height(view.getStorage().getTextureHeight())
{
#ifdef BEATMUP_DEBUG
    Storage::checkChannelNumber(channel);
    OutOfRange::check(channel, 0, view.getDepth() - 1, "Channel index out of range: %d");
#endif
    const int storageChannel = view.channels[ channel/4 ].channelIdx;
    const int storageTexture = view.storage->getChannelTextureNumber(storageChannel);
    GL::TextureHandler::textureHandle = view.storage->textures[storageTexture].handle;
}


Storage::TextureHandler::~TextureHandler() {
    GL::TextureHandler::textureHandle = 0;
        // dropping the texture handle; the texture is not owned and is managed by the Storage
}


void Storage::TextureHandler::prepare(GraphicPipeline& gpu) {
    glBindTexture(GL_TEXTURE_2D, GL::TextureHandler::textureHandle);
}


bool Storage::Binder::begin(GL::Program& program, Storage::View& output, int channel) {
#ifdef BEATMUP_DEBUG
    Storage::checkChannelNumber(channel);
    DebugAssertion::check(output.storage->textures, "Output storage is not allocated on GPU");
#endif

    // reset counter
    unit = 0;

    // check if previous binding can be reused
    bool fast = this->program == &program;

    // setup program if needed and store
    if (!fast) {
        program.enable(gpu);
        this->program = &program;
    }

    // check if previous output setting is okay
    const int outTexture = output.textures[output.getChannelTextureNumber(channel)];
    Storage& storage = *output.storage;

    fast = fast && this->outputTexture == storage.textures[outTexture].handle;

    // setup output
    if (!fast) {
        gpu.bindOutput(storage.textures[outTexture].handle);
        if (storage.textures[outTexture].dirty) {
            glClear(GL_COLOR_BUFFER_BIT);
            storage.textures[outTexture].dirty = false;
        }

        this->outputTexture = storage.textures[outTexture].handle;
    }

    // fix output area
    IntPoint origin = storage.getChannelOrigin(output.channels[channel / 4].channelIdx);
    glViewport(origin.getX(), origin.getY(), output.getWidth(), output.getHeight());

    storage.upToDate[ProcessingTarget::CPU] = false;

    return fast;
}


void Storage::Binder::begin(GL::Program& program, AbstractBitmap& output) {
    unit = 0;
    program.enable(gpu);
    gpu.bindOutput(output);
    this->program = &program;
    this->outputTexture = 0;
}


void Storage::Binder::operator()(Storage::View& input, const char* name) {
    if (!input.storage->upToDate[ProcessingTarget::GPU])
        input.storage->push(gpu);

    const int num = input.getNumberOfTextures();
    program->setIntegerArray(name, unit, num);
    for (int i = 0; i < num; ++i, ++unit)
        bind(unit, input.storage->textures[ input.textures[i] ].handle);
    GL::GLException::check("binding input storage");
}


void Storage::Binder::operator()(Storage::View& input, const char* name, int channel) {
#ifdef BEATMUP_DEBUG
    Storage::checkChannelNumber(channel);
    OutOfRange::check(channel, 0, input.getDepth() * 4 - 1, "Channel index is out of range");
#endif
    if (!input.storage->upToDate[ProcessingTarget::GPU])
        input.storage->push(gpu);

    program->setIntegerArray(name, unit, 1);
    const int storageChannel = input.channels[channel/4].channelIdx;
    const int storageTexture = input.storage->getChannelTextureNumber(storageChannel);
    bind(unit, input.storage->textures[storageTexture].handle);
    ++unit;
    GL::GLException::check("binding input storage");
}


void Storage::Binder::operator()(GL::TextureHandler& input, const char* name) {
    gpu.bind(input, unit, TextureParam::INTERP_NEAREST);
    program->setInteger(name, unit++);
}


Storage::Scanner::~Scanner() {
    if (ptr)
        delete[] ptr;
}


void Storage::Scanner::bind(Storage::View& view) {
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(!this->view, "A view is already bound");
    DebugAssertion::check(view.storage->memory, "Storage is not allocated in RAM");
#endif
    // set internal state
    this->view = &view;
    if (view.channels.size() != ptrSize || !ptr) {
        if (ptr)
            delete[] ptr;
        ptrSize = view.channels.size();
        ptr = new sample_t*[ptrSize];
    }

    // acquire memory
    data = view.storage->memory.ptr<sample_t>();
}


void Storage::Scanner::unbind() {
    if (view)
        view = nullptr;
}


void Storage::Scanner::move(int x, int y) {
    const Storage& storage = *view->storage;
    const int
        w = storage.getTextureWidth(),
        h = storage.getTextureHeight();
    for (size_t i = 0; i < ptrSize; ++i) {
        const IntPoint pos = view->getChannelOrigin(4 * i) + IntPoint(x, y);
        const int storageTexNum = view->textures[view->channels[i].textureIdx];
        ptr[i] = data + ((storageTexNum * h + pos.y) * w + pos.x);
    }
}


Storage::Scanner& Storage::Scanner::operator++() {
    size_t i = 0;
#ifdef BEATMUP_ENABLE_NEON
    if (sizeof(void*) == 8) {
        i += ptrSize / 2 * 2;
        auto _1 = vdupq_n_u64(sizeof(sample_t*));
        uint64_t* p = (uint64_t*)ptr;
        const uint64_t* stop = p + i;
        for (; p < stop; p += 2)
            vst1q_u64(p, vaddq_u64(vld1q_u64(p), _1));
    }
    else if (sizeof(void*) == 4) {
        i += ptrSize / 4 * 4;
        auto _1 = vdupq_n_u32(sizeof(sample_t*));
        uint32_t* p = (uint32_t*)ptr;
        const uint32_t* stop = p + i;
        for (; p < stop; p += 4)
            vst1q_u32(p, vaddq_u32(vld1q_u32(p), _1));
    }
#endif
    for (; i < ptrSize; ++i)
        ++ptr[i];

    return *this;
}


Size::Padding NNets::paddingFromString(const std::string& str) {
    const std::string lc = StringUtils::lowercase(str);
    if (lc == "valid")
        return Size::Padding::VALID;
    if (lc == "same")
        return Size::Padding::SAME;
    throw InvalidArgument("Invalid padding: " + str);
    return Size::Padding::VALID;
}


std::string std::to_string(const Size::Padding& padding) {
    switch (padding) {
        case Size::Padding::SAME: return "same";
        case Size::Padding::VALID: return "valid";
    }
    Insanity::insanity("Invalid padding");
    return "";
}