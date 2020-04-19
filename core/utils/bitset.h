/**
    Stuff
 */
#pragma once
#include <stdint.h>
#include "../basic_types.h"
namespace Beatmup {
    namespace Utils {
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
            msize size;
        public:
            inline Bitset(): size(0) {}
            inline Bitset(msize size, bool value) : bits(ceili(size, PACK_SIZE), value ? ALL_ONES : 0), size(size)
            {}
            inline void resize(msize size) {
                bits.resize(ceili(size, PACK_SIZE));
                this->size = size;
            }
            inline void setAll(bool value) {
                if (value)
                    for (auto &_ : bits) _ = ALL_ONES;
                else
                    for (auto &_ : bits) _ = 0;
            }
            inline void set(msize i, bool value) {
                BEATMUP_ASSERT_DEBUG(i < size);
                auto& _ = bits[i / PACK_SIZE];
                _ = value
                    ? _ |  (_1_ << (i & (PACK_SIZE - 1)))
                    : _ & ~(_1_ << (i & (PACK_SIZE - 1)));
            }
            inline bool all() const {
                for (msize i = 0; i < size / PACK_SIZE; ++i)
                    if (bits[i] != ALL_ONES)
                        return false;
                const msize rem = size & (PACK_SIZE - 1);
                if (rem > 0)
                    return (~(bits.back() & ((_1_ << rem) - 1))) == 0;
                return true;
            }
            inline bool any() const {
                for (msize i = 0; i < size / PACK_SIZE; ++i)
                    if (bits[i] != 0)
                        return false;
                const msize rem = size & (PACK_SIZE - 1);
                if (rem > 0)
                    return (bits.back() & ((_1_ << rem) - 1)) > 0;
                return true;
            }
            inline bool operator[](msize i) const {
                BEATMUP_ASSERT_DEBUG(i < size);
                return (
                    bits[i / PACK_SIZE] & (_1_ << (i & (PACK_SIZE - 1)))
                ) > 0;
            }
        };
    }}