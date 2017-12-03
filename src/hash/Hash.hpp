
#ifndef SIMPLEDB_HASH_HASH_HPP
#define SIMPLEDB_HASH_HASH_HPP

#include "MurmurHash3.h"

#include <cstddef>
#include <cstdint>

namespace simpledb {

    struct Hash {
        size_t operator()(const void* key, uint64_t len) const;
        size_t operator()(const void* key, uint64_t len, uint32_t seed) const;

    private:
        static constexpr const uint32_t DEFAULT_SEED = 0x27c110b5;
    };
}

#endif	/* SIMPLEDB_HASH_HASH_HPP */
