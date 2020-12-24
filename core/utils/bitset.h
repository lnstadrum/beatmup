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
#include <cstdint>
#include "../basic_types.h"
#include <vector>


namespace Beatmup {

    /**
       A set of boolean flags.
    */
    class Bitset {
    private:
#ifdef BEATMUP_PLATFORM_64BIT_
        typedef uint64_t bits_t;
        static const bits_t ALL_ONES = 0xffffffffffffffff;
#else
        typedef uint32_t bits_t;
        static const bits_t ALL_ONES = 0xffffffff;
#endif

        static const bits_t _1_ = (bits_t)1;
        static const size_t PACK_SIZE = 8 * sizeof(bits_t);
        std::vector<bits_t> bits;
        size_t size;

        inline bool getBit(size_t i) const {
            return ( bits[i / PACK_SIZE] & (_1_ << (i & (PACK_SIZE - 1))) ) > 0;
        }

    public:
        inline Bitset(): size(0) {}

        inline Bitset(size_t size, bool value) : bits(ceili(size, PACK_SIZE), value ? ALL_ONES : 0), size(size)
        {}

        inline void resize(size_t size) {
            bits.resize(ceili(size, PACK_SIZE));
            this->size = size;
        }

        inline void setAll(bool value) {
            if (value)
                for (auto &_ : bits) _ = ALL_ONES;
            else
                for (auto &_ : bits) _ = 0;
        }

        inline void set(size_t i, bool value = true) {
            BEATMUP_ASSERT_DEBUG(i < size);
            auto& _ = bits[i / PACK_SIZE];
            _ = value
                ? _ |  (_1_ << (i & (PACK_SIZE - 1)))
                : _ & ~(_1_ << (i & (PACK_SIZE - 1)));
        }

        inline bool all() const {
            for (size_t i = 0; i < size / PACK_SIZE; ++i)
                if (bits[i] != ALL_ONES)
                    return false;
            const size_t rem = size & (PACK_SIZE - 1);
            if (rem > 0)
                return (~(bits.back() & ((_1_ << rem) - 1))) == 0;
            return true;
        }

        inline bool any() const {
            for (size_t i = 0; i < size / PACK_SIZE; ++i)
                if (bits[i] != 0)
                    return false;
            const size_t rem = size & (PACK_SIZE - 1);
            if (rem > 0)
                return (bits.back() & ((_1_ << rem) - 1)) > 0;
            return true;
        }

        inline size_t count() const {
            size_t n = 0;
            for (size_t i = 0; i < size; ++i)
                if (getBit(i))
                    n++;
            return n;
        }

        inline bool operator[](size_t i) const {
            BEATMUP_ASSERT_DEBUG(i < size);
            return getBit(i);
        }
    };
}
