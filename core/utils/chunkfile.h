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

#pragma once
#include "../exception.h"
#include "input_stream.h"
#include <fstream>
#include <map>
#include <vector>
#include <cstdint>


namespace Beatmup {
    typedef uint32_t chunksize_t;

    /**
        A key-value pair set storing pieces of arbitrary data (chunks) under string keys.
        A chunk is a header and a piece of data packed in memory like this: (idLength[4], id[idLength], size[sizeof(chunksize_t)], data[size])
        ChunkCollection defines an interface to retrieve chunks by their ids.
    */
    class ChunkCollection : public Object {
    public:
        /**
            Opens the collection to read chunks from it.
        */
        virtual void open() = 0;

        /**
            Closes the collection after a reading session.
        */
        virtual void close() = 0;

        /**
            Returns the number of chunks available in the collection after it is opened.
        */
        virtual size_t size() const = 0;

        /**
            Check if a specific chunk exists.
            \param[in] id       The chunk id
            \return `true` if the chunk exists in the collection.
        */
        virtual bool chunkExists(const std::string& id) const = 0;

        /**
            Retrieves size of a specific chunk.
            \param[in] id       The chunk id
            \return size of the chunk in bytes, 0 if not found.
        */
        virtual chunksize_t chunkSize(const std::string& id) const = 0;

        /**
            Reads a chunk.
            The collection is expected to be opened.
            \param[in] id       Wanted chunk id.
            \param[out] data    A buffer to write out the wanted chunk content.
            \param[in] limit    The buffer capacity in bytes.
            \return number of bytes written out to the buffer:
                * if the chunk is found fits the buffer, the chunk size is returned;
                * if the chunk is found and too big, \p limit is returned (number of bytes actually written);
                * if no chunk found, 0 is returned.
        */
        virtual chunksize_t fetch(const std::string& id, void* data, const chunksize_t limit) = 0;

        /**
            Saves the collection to a file.
            \param[in] filename     The name of the file to write chunks to
            \param[in] append       If `true`, writing to the end of the file (keeping the existing content). Rewriting the file otherwise.
        */
        virtual void save(const std::string& filename, bool append = false) = 0;

        /**
            Reads a chunk and casts it into a given type.
            \param[in] id       The searched chunk id.
        */
        template<typename datatype> inline datatype read(const std::string& id) {
            datatype result;
#ifdef BEATMUP_DEBUG
            DebugAssertion::check(sizeof(result) == chunkSize(id),
                "Cannot fit chunk " + id + " of " + std::to_string(chunkSize(id)) + " bytes into " + std::to_string(sizeof(result)) + " bytes");
#endif
            fetch(id, &result, sizeof(result));
            return result;
        }

        /**
            Reads a chunk into a vector of a specific type.
            \param[in] id       The searched chunk id.
        */
        template<typename datatype>
        inline std::vector<datatype> readVector(const std::string& id) {
#ifdef BEATMUP_DEBUG
            DebugAssertion::check(chunkSize(id) % sizeof(datatype) == 0,
                "Cannot read chunk " + id + " of " + std::to_string(chunkSize(id)) + " bytes in a vector of elements"
                " of " + std::to_string(sizeof(datatype)) + " bytes each");
#endif
            std::vector<datatype> result;
            result.resize(chunkSize(id) / sizeof(datatype));
            fetch(id, static_cast<void*>(result.data()), result.size() * sizeof(datatype));
            return result;
        }

    };

    template<>
    inline std::string ChunkCollection::read(const std::string& id) {
        std::string result;
        result.resize(chunkSize(id));
        fetch(id, const_cast<char*>(result.data()), result.size());
        return result;
    }

    /**
        Stream of chunks.
        Implements ChunkCollection from a streamed source.
    */
    class ChunkStream : public ChunkCollection {
    private:
        typedef struct {
            chunksize_t size;
            chunksize_t pos;
        } ChunkDesc;

        std::map<std::string, ChunkDesc> map;
        InputStream& stream;

    protected:
        inline ChunkStream(InputStream& stream): stream(stream) {}

        /**
            Goes through the input stream to build the list of existing chunks.
            Does not read the chunks content, only headers.
        */
        bool parse();

    public:
        inline size_t size() const { return map.size(); }

        inline bool chunkExists(const std::string& id) const { return map.find(id) != map.end(); }

        chunksize_t chunkSize(const std::string& id) const;

        chunksize_t fetch(const std::string& id, void* data, const chunksize_t limit);

        void save(const std::string& filename, bool append = false);
    };

    /**
        File containing chunks.
        The file is not loaded in memory, but is scanned when first opened to collect the information about available chunks.
    */
    class ChunkFile : public ChunkStream {
    private:
        const std::string filename;
        FileInputStream stream;
    public:
        /**
            Returns `true` if a given file is readable
        */
        static bool readable(const std::string& filename);

        /**
            Creates a read-only chunk collection from a file.
            \param[in] filename     The file name / path
            \param[in] openNow      If `true`, the file is read right away. Otherwise it is done on open() call.
                                    No information is available about chunks in the file until it is opened.
        */
        ChunkFile(const std::string& filename, bool openNow = true);
        void open();
        void close();
    };

    /**
        Writes chunks to a file
    */
    class ChunkFileWriter {
    private:
        std::fstream stream;
    public:
        ChunkFileWriter(const std::string& filename, bool append = false);
        ~ChunkFileWriter();

        void operator()(const std::string& id, const void* data, const chunksize_t size);

        template<typename datatype> void operator()(const std::string& id, datatype something) {
            (*this)(id, &something, sizeof(something));
        }
    };

    /**
        Simply a piece of binary data of a specific size.
        A brick in ChunkCollection-based data sets.
        No information is stored on the nature of the data: it is up to the application to interpret the chunk content.
    */
    class Chunk {
    private:
        size_t chunkSize;
        void* data;
    public:
        /**
            Makes an empty chunk.
        */
        inline Chunk(): chunkSize(0), data(nullptr) {}

        /**
            Allocates a chunk of a given size.
            \param[in] size    Chunk size in bytes
        */
        Chunk(size_t size);

        /**
            Reads a chunk from a collection.
            \param[in] collection   The collection to read from
            \param[in] id           The chunk id to find in the collection
        */
        Chunk(ChunkCollection& collection, const std::string& id);

        ~Chunk();

        inline Chunk(Chunk&& chunk): chunkSize(chunk.chunkSize), data(chunk.data) {
            chunk.data = nullptr;
            chunk.chunkSize = 0;
        }

        inline Chunk& operator=(Chunk&& chunk) {
            if (data)
                free(data);
            chunkSize = chunk.chunkSize;
            data = chunk.data;
            chunk.data = nullptr;
            chunk.chunkSize = 0;
            return *this;
        }

        /**
            Writes a chunk out to a file.
            \param[in] file    The file to write to
            \param[in] id      The chunk id
        */
        void writeTo(ChunkFileWriter& file, const std::string& id) const;

        inline size_t size() const { return chunkSize; }

        inline void* operator()() { return data; }
        inline const void* operator()() const { return data; }

        inline operator bool() const { return data != nullptr; }

        template<typename datatype> inline datatype* ptr(size_t offset = 0) {
            return (datatype*)(data) + offset;
        }
        template<typename datatype> inline const datatype* ptr(size_t offset = 0) const {
            return (datatype*)(data) + offset;
        }

        template<typename datatype> inline datatype at(size_t offset) const {
            return ((datatype*)data)[offset];
        }

        template<typename datatype> inline datatype& at(size_t offset) {
            return ((datatype*)data)[offset];
        }
    };
}
