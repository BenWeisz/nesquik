#ifndef NESQUIK_HASH_H
#define NESQUIK_HASH_H

#include "types.h"

#define HASH_FNV32_BASIS 0x811c9dc5
#define HASH_FNV32_PRIME 0x01000193

u32 HASH_fnv1a(const u8* data, u32 size);

#endif //NESQUIK_HASH_H