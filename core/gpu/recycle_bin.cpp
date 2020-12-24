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

#include "recycle_bin.h"
#include "../parallelism.h"
#include "../debug.h"

using namespace Beatmup;
using namespace GL;


namespace Internal {
    /**
        A task to empty the recycle bin
    */
    class Recycler : public AbstractTask {
    private:
        std::vector<RecycleBin::Item*>& items;

    public:
        Recycler(std::vector<RecycleBin::Item*>& items) : items(items) {}


        TaskDeviceRequirement getUsedDevices() const {
            return TaskDeviceRequirement::GPU_OR_CPU;
        }


        bool process(TaskThread& thread) {
            return true;
        }


        bool processOnGPU(GraphicPipeline& gpu, TaskThread& thread) {
            for (auto& item : items) {
                RecycleBin::Item* deleting = item;
                item = nullptr;
                if (deleting)
                    delete deleting;
            }
            items.clear();
            return true;
        }
    };
}


RecycleBin::RecycleBin(Context& ctx) : ctx(ctx) {
    recycler = new Internal::Recycler(items);
}


RecycleBin::~RecycleBin() {
    delete recycler;
}


void RecycleBin::put(RecycleBin::Item* item) {
    if (item) {
        lock();
        items.push_back(item);
        unlock();
    }
}


void RecycleBin::put(std::initializer_list<Item*> items) {
    lock();
    for (auto item : items)
        if (item)
            this->items.push_back(item);
    unlock();
}


void RecycleBin::emptyBin() {
    lock();
    if (items.size() > 0)
        ctx.performTask(*recycler);
    unlock();
}