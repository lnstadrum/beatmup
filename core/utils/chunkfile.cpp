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

#include "chunkfile.h"

using namespace Beatmup;

typedef uint32_t id_size_t;     //!< chunk id length type


class ChunkIdTooLong : public Exception {
public:
    ChunkIdTooLong(const std::string& id) :
        Exception("Chunk id exceeds max allowed length (255 chars): %s", id.c_str()) {}

    static void check(const std::string& id) {
        if (id.size() > 255)
            throw ChunkIdTooLong(id);
    }
};


bool ChunkStream::parse() {
    map.clear();
    if (!stream.seek(0))
        return false;

    size_t pos = 0;
    id_size_t idLength;
    while (stream(&idLength, sizeof(id_size_t))) {
        // read id
        std::string id(idLength, 0);
        if (!stream(&id[0], idLength))
            return false;
        pos += idLength + sizeof(id_size_t);

        // read length
        ChunkDesc chunkDesc;
        if (!stream(&chunkDesc.size, sizeof(chunksize_t)))
            return false;
        pos += sizeof(chunksize_t);

        // read content
        chunkDesc.pos = pos;
        map[id] = chunkDesc;
        pos += chunkDesc.size;
        if (!stream.seek(pos))
            return false;
    }
    return true;
}


chunksize_t ChunkStream::chunkSize(const std::string& id) const {
    const auto& chunk = map.find(id);
    return chunk == map.end() ? 0 : chunk->second.size;
}


chunksize_t ChunkStream::fetch(const std::string& id, void* data, const chunksize_t limit) {
#ifdef BEATMUP_DEBUG
    ChunkIdTooLong::check(id);
#endif

    const auto& chunk = map.find(id);
    if (chunk == map.end())
        return 0;

    if (!stream.seek(chunk->second.pos))
        throw RuntimeError("Cannot seek for chunk " + id);

    const chunksize_t size = chunk->second.size < limit ? chunk->second.size : limit;
    if (!stream(data, size))
        throw RuntimeError("Cannot read chunk " + id);

    return size;
}


void ChunkStream::save(const std::string& filename, bool append) {
    ChunkFileWriter writer(filename, append);
    for (auto it : map) {
        Chunk chunk(*this, it.first);
        writer(it.first, chunk(), chunk.size());
    }
}


bool ChunkFile::readable(const std::string& filename) {
    std::ifstream stream(filename, std::fstream::in | std::fstream::binary);
    return stream.good();
}


ChunkFile::ChunkFile(const std::string& filename, bool openNow) :
    ChunkStream(stream),
    filename(filename)
{
    if (openNow)
        open();
}


void ChunkFile::open() {
    stream.open(filename.c_str());
    stream.clear();
    parse();
    stream.clear();
}

void ChunkFile::close() {
    stream.close();
}


ChunkFileWriter::ChunkFileWriter(const std::string& filename, bool append) :
    stream(filename, std::ios::binary | (append ? std::ios::out | std::ios::app | std::ios::ate : std::ios::out))
{
    if (!stream.good())
        throw IOError(filename, "Cannot write to file");
}


ChunkFileWriter::~ChunkFileWriter() {}


void ChunkFileWriter::operator()(const std::string& id, const void* data, const chunksize_t size) {
    ChunkIdTooLong::check(id);
    id_size_t idLen = (char)id.size();
    stream.write((char*)&idLen, sizeof(idLen));
    stream.write(&id[0], idLen);
    stream.write((char*)&size, sizeof(size));
    stream.write((char*)data, size);
}


Chunk::Chunk(size_t size) : chunkSize(size) {
    data = malloc(size);
}


Chunk::Chunk(ChunkCollection& collection, const std::string& id):
    Chunk(collection.chunkSize(id))
{
#ifdef BEATMUP_DEBUG
    DebugAssertion::check(collection.chunkExists(id), "Chunk not found: " + id);
#endif
    collection.fetch(id, data, chunkSize);
}


void Chunk::writeTo(ChunkFileWriter& file, const std::string& id) const {
    file(id, data, chunkSize);
}


Chunk::~Chunk() {
    free(data);
}
