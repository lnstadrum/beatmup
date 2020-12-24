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

#include "asset.h"
#include <cstdio>

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
    if (!asset)
        throw IOError(path, "Cannot access the asset");
}

Asset::~Asset() {
    AAsset_close(asset);
}

bool Asset::operator()(void *buffer, msize bytes) {
    if (bytes == 0)
        return true;
    return AAsset_read(asset, buffer, bytes) > 0;
}

bool Asset::seek(msize pos) {
    auto result = AAsset_seek64(asset, pos, SEEK_SET);
    return result != (off64_t) -1;
}

bool Asset::eof() const {
    return AAsset_getRemainingLength(asset) <= 0;
}


ChunkAsset::ChunkAsset(AAssetManager* manager, const std::string& filename):
    stream(manager, filename.c_str()),
    ChunkStream(stream)
{
    if (!parse())
        throw IOError(filename, "Cannot parse asset");
}