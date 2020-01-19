#pragma once

#include "../input_stream.h"
#include <android/asset_manager.h>
#include <vector>
#include <string>

namespace Beatmup {
    namespace Android {

        class AssetPath {
        private:
            AAssetManager *manager;
            std::vector<AAssetDir*> path;
        public:
            static const char PATH_DELIMITER = '/';

            AssetPath(AAssetManager* manager, const char* path = "");
            ~AssetPath();

            void follow(const char* path);
            bool up();
            msize listFiles(std::vector<std::string>& files);
        };


        class Asset : public InputStream {
        private:
            AAsset* asset;
        public:
            Asset(AAssetManager* manager, const char* path);
            ~Asset();

            bool operator()(void * buffer, msize bytes);
            bool eof() const;
        };

    }
}