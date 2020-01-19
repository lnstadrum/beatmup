#include "asset.h"
#include "../../debug.h"

using namespace Beatmup;
using namespace Android;

AssetPath::AssetPath(AAssetManager *manager, const char *path_): manager(manager) {
    follow(path_);
}

AssetPath::~AssetPath() {
    for (auto _ = path.rbegin(); _ != path.rend(); ++_) {
        AAssetDir_close(*_);
    }
}

void AssetPath::follow(const char *path_) {
    size_t pos = 0;
    std::string path(path_);
    do {
        size_t i = path.find(PATH_DELIMITER, pos);
        if (i == std::string::npos)
            i = path.length() + 1;
        std::string dir(path.substr(pos, i - pos - 1));
        if (dir == "..")
            up();
        else
            this->path.push_back( AAssetManager_openDir(manager, dir.c_str()) );
        pos = i + 1;
    } while (pos < path.length());
}

msize AssetPath::listFiles(std::vector<std::string> &files) {
    if (path.empty())
        return (msize)0;
    const char* filename = nullptr;
    msize counter = 0;
    while (( filename = AAssetDir_getNextFileName(path.back()) ) != nullptr) {
        files.emplace_back(filename);
        counter++;
    }
    AAssetDir_rewind(path.back());
    return counter;
}

bool AssetPath::up() {
    if (path.size() <= 1)
        return false;
    AAssetDir_close(path.back());
    path.pop_back();
    return true;
}


Asset::Asset(AAssetManager *manager, const char *path) {
    asset = AAssetManager_open(manager, path, AASSET_MODE_STREAMING);
}

Asset::~Asset() {
    AAsset_close(asset);
}

bool Asset::operator()(void *buffer, msize bytes) {
    return AAsset_read(asset, buffer, bytes) > 0;
}

bool Asset::eof() const {
    return AAsset_getRemainingLength(asset) <= 0;
}
