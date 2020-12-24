/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include <pybind11/pytypes.h>
#include <map>
#include <string>

#include "utils/chunkfile.h"
#include "context.h"

namespace Beatmup {
    namespace Python {

        /**
            Writable ChunkCollection implementation for Python.
            Allows to exchange binary data without copying.
        */
        class WritableChunkCollection : public ChunkCollection {
        private:
            std::map<std::string, pybind11::buffer> data;

        public:
            WritableChunkCollection() {}

            inline pybind11::buffer& operator[](const std::string& id) {
                return data[id];
            }

            inline void open() {}
            inline void close() {}

            inline size_t size() const { return data.size(); }

            inline bool chunkExists(const std::string& id) const { return data.find(id) != data.cend(); }

            chunksize_t chunkSize(const std::string& id) const;

            chunksize_t fetch(const std::string& id, void* data, const chunksize_t limit);

            void save(const std::string& filename, bool append = false);
        };

    }
}