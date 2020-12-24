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
#include <mutex>
#include <vector>
#include "../context.h"
#include "../utils/lockable_object.h"
#include <initializer_list>

namespace Beatmup {
    namespace GL {

        /**
            Stores references to GPU resources that will not be used anymore and needed to be recycled in a thread having access to the GPU.
            GPU resources such as texture handles generally cannot be destroyed in a user thread due to OpenGL API restrictions.
            They are kept in a RecycleBin until the application empties it, which launches a task running in the thread that can access the GPU.
        */
        class RecycleBin : public LockableObject {
        public:
            /**
                A wrapper for a GPU resource. Destroyed in a GPU-aware thread when emptying the recycle bin.
            */
            class Item {
            private:
                Item(const Item&) = delete;		//!< disabling copying constructor
            public:
                Item() {}
                virtual ~Item() {}
            };

        private:
            std::vector<Item*> items;
            Context& ctx;
            AbstractTask* recycler;

        public:
            RecycleBin(Context& ctx);
            ~RecycleBin();

            /**
                Puts an item into the recycle bin
            */
            void put(Item* item);

            /**
                Puts items into the recycle bin
            */
            void put(std::initializer_list<Item*> items);

            /**
                Empty the bin destroying all the items in a GPU-aware thread
            */
            void emptyBin();
        };
    }
}
