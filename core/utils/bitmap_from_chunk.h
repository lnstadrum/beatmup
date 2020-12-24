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
#include "../parallelism.h"
#include "../bitmap/abstract_bitmap.h"
#include "../bitmap/content_lock.h"
#include "chunkfile.h"

namespace Beatmup {

    /**
        Loads a bitmap content from chunk.
        The bitmap needs to be created in advance. The loaded chunk contains pixel data in binary form.
    */
    class BitmapFromChunk : public AbstractTask, public BitmapContentLock {
    private:
        virtual bool process(TaskThread& thread);
        virtual void beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu);
        virtual void afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted);
        AbstractBitmap* bitmap;
        ChunkCollection* collection;
        std::string chunkId;
    public:
        inline BitmapFromChunk(): bitmap(nullptr), collection(nullptr) {}
        inline BitmapFromChunk(AbstractBitmap& bitmap, ChunkCollection& collection, const std::string& chunkId):
            bitmap(&bitmap), collection(&collection), chunkId(chunkId) {}

        inline void setBitmap(AbstractBitmap* bitmap) { this->bitmap = bitmap; }

        inline void setCollection(ChunkCollection* collection) { this->collection = collection; }

        inline void setChunkId(const std::string& chunkId) { this->chunkId = chunkId; }

        /**
            Loads a bitmap content from a given collection and chunk.
            Launches a task in the main thread pool of the bitmap context.
            \param[in,out] bitmap       The bitmap to load
            \param[in,out] collection   The collection to load from
            \param[in,out] chunkId      Chunk ID containing the content to load
        */
        static void load(AbstractBitmap& bitmap, ChunkCollection& collection, const std::string& chunkId);
    };
}