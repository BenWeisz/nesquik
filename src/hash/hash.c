#include "hash/hash.h"

u32 HASH_fnv1a(const u8* data, const u32 size) {
    u32 hash = HASH_FNV32_BASIS;
    for (u32 i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= HASH_FNV32_PRIME;
    }

    return hash;
}