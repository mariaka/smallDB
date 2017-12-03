
#include "Hash.hpp"

namespace simpledb {

    size_t Hash::operator()(const void* key, uint64_t len) const {
        return operator()(key, len, DEFAULT_SEED);
    }

    size_t Hash::operator()(const void* key, uint64_t len, uint32_t seed) const {
#if defined(__i386) || defined(_M_IX86) // x86 32-bit
        if (sizeof (size_t) <= 4) {
            uint32_t hash;
            MurmurHash3_x86_32(key, len, seed, &hash);
            return *reinterpret_cast<size_t*> (&hash);
        } else {
            uint64_t hash[2];
            MurmurHash3_x86_128(key, len, seed, hash);
            return *reinterpret_cast<size_t*> (&hash[0]);
        }
        //#elif defined(__x86_64__) || defined(_M_X64) // x86 64-bit
#else
        uint64_t hash[2];
        MurmurHash3_x64_128(key, len, seed, hash);
        return *reinterpret_cast<size_t*> (&hash[0]);
#endif
    }
}
