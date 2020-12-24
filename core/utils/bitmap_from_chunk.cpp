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

#include "bitmap_from_chunk.h"

using namespace Beatmup;

bool BitmapFromChunk::process(TaskThread& thread) {
    collection->fetch(chunkId, bitmap->getData(0, 0), bitmap->getMemorySize());
    return true;
}


void BitmapFromChunk::beforeProcessing(ThreadIndex threadCount, ProcessingTarget target, GraphicPipeline* gpu) {
    NullTaskInput::check(bitmap, "bitmap");
    NullTaskInput::check(collection, "chunk collection");
    InvalidArgument::check(collection->chunkExists(chunkId), "Chunk not found: " + chunkId);
    const size_t chunkSize = collection->chunkSize(chunkId);
    const size_t bitmapSize = bitmap->getMemorySize();
    InvalidArgument::check(chunkSize == bitmapSize,
        "Chunk size does not match bitmap size: " + std::to_string(chunkSize) + " vs "  + std::to_string(bitmapSize) + " bytes");
    writeLock(gpu, bitmap, ProcessingTarget::CPU);
}


void BitmapFromChunk::afterProcessing(ThreadIndex threadCount, GraphicPipeline* gpu, bool aborted) {
    unlock(bitmap);
}


void BitmapFromChunk::load(AbstractBitmap& bitmap, ChunkCollection& collection, const std::string& chunkId) {
    BitmapFromChunk task(bitmap, collection, chunkId);
    bitmap.getContext().performTask(task);
}