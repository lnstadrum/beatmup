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

#include "../input_stream.h"
#include "../chunkfile.h"
#include <android/asset_manager.h>
#include <vector>
#include <string>

namespace Beatmup {
    namespace Android {

        /**
            Path to assets in Android
            Used to reach a specific location.
        */
        class AssetPath {
        private:
            AAssetManager *manager;
            std::vector<AAssetDir*> path;
        public:
            static const char PATH_DELIMITER = '/';

            AssetPath(AAssetManager* manager, const char* path = "");
            ~AssetPath();

            void follow(const char* path);

            /**
                Goes one level up ("..") from the current asset folder
            */
            bool up();

            /**
                Lists files in the current folder
                \param[out] files       A list of strings the names of files in the current folder are appended to
                \return number of files in the current folder.
            */
            msize listFiles(std::vector<std::string>& files);
        };


        /**
            Android assets reader.
            Implements InputStream interface enabling access to Android assets.
        */
        class Asset : public InputStream {
        private:
            AAsset* asset;
        public:
            Asset(AAssetManager* manager, const char* path);
            ~Asset();

            bool operator()(void * buffer, msize bytes);
            bool seek(msize pos);
            bool eof() const;
        };


        /**
            Asset containing chunks.
        */
        class ChunkAsset : public ChunkStream {
        private:
            Asset stream;
        public:
            /**
                Creates a read-only chunk collection from an asset.
                \param[in] manager      AAssetManager instance
                \param[in] filename     The asset file name / path
            */
            ChunkAsset(AAssetManager* manager, const std::string& filename);
            inline void open() {}
            inline void close() {}
        };
    }
}