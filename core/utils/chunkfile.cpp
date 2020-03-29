#include "chunkfile.h"

using namespace Beatmup;


void ChunkFile::ChunkIdTooLong::check(const std::string& id) {
    if (id.size() > 255)
        throw ChunkIdTooLong(id);
}


bool ChunkFile::readable(const std::string& filename) {
    std::ifstream stream(filename, std::fstream::in | std::fstream::binary);
    return stream.good();
}


ChunkFile::ChunkFile(const std::string& filename) :
    filename(filename)
{}


ChunkFile::~ChunkFile() {
    stream.close();
}


void ChunkFile::open(bool forceUpdateMap) {
    stream.open(filename, std::fstream::in | std::fstream::binary);
    if (!map.empty() && !forceUpdateMap)
        return;

    if (!stream.good())
        throw ChunkFileAccessError(filename);

    map.clear();
    uint8_t idLength;
    while (stream.read((char*)&idLength, 1)) {
        std::string id(idLength, 0);
        stream.read(&id[0], idLength);
        ChunkDesc chunkDesc;
        stream.read((char*)&chunkDesc.size, sizeof(chunksize));
        chunkDesc.pos = stream.tellg();
        map[id] = chunkDesc;
        stream.seekg(chunkDesc.size, std::fstream::cur);
    }
    stream.clear();
}


void ChunkFile::close() {
    stream.close();
}


ChunkFile::chunksize ChunkFile::chunkSize(const std::string& id) const {
    const auto& chunk = map.find(id);
    return chunk == map.end() ? 0 : chunk->second.size;
}


ChunkFile::chunksize ChunkFile::fetch(const std::string& id, void* data, const chunksize limit) {
    BEATMUP_ASSERT_DEBUG(stream.is_open());

#ifdef BEATMUP_DEBUG
    ChunkIdTooLong::check(id);
#endif

    const auto& chunk = map.find(id);
    if (chunk == map.end())
        return 0;

    stream.seekg(chunk->second.pos, std::fstream::beg);
    if (!stream.good())
        throw ChunkFileAccessError(filename);

    const chunksize size = chunk->second.size < limit ? chunk->second.size : limit;
    stream.read((char*)data, size);

    return size;
}


ChunkFile::Writer::Writer(const std::string& filename, bool append) :
    stream(filename, std::ios::binary | (append ? std::ios::out | std::ios::app | std::ios::ate : std::ios::out))
{
    if (!stream.good())
        throw ChunkFileAccessError(filename, false);
}


ChunkFile::Writer::~Writer() {
    stream.close();
}


void ChunkFile::Writer::operator()(const std::string& id, const void* data, const chunksize size) {
    ChunkIdTooLong::check(id);
    char idLen = (char)id.size();
    stream.write(&idLen, sizeof(idLen));
    stream.write(&id[0], idLen);
    stream.write((char*)&size, sizeof(size));
    stream.write((char*)data, size);
}


Chunk::Chunk(size_t size) : chunkSize(size) {
    data = malloc(size);
}


Chunk::Chunk(ChunkFile& file, const std::string& id):
    Chunk(file.chunkSize(id))
{
    file.fetch(id, data, chunkSize);
}


void Chunk::writeTo(ChunkFile::Writer& file, const std::string& id) const {
    file(id, data, chunkSize);
}


Chunk::~Chunk() {
    free(data);
}
