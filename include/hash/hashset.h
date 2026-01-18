#ifndef NESQUIK_HASHSET_H
#define NESQUIK_HASHSET_H

#include "types.h"

#define HASHSET_ENTRY_STATUS_EMPTY      0
#define HASHSET_ENTRY_STATUS_FILLED     1
#define HASHSET_ENTRY_STATUS_TOMBSTONE  2

#define HASHSET_MIN_LOAD_FACTOR         0.5
#define HASHSET_MAX_LOAD_FACTOR         0.75

#define HASHSET_MIN_CAPACITY            8

#pragma pack(push, 1)
typedef struct HASHSET_ENTRY {
    u8 status;
    u32 hash;
    // These need to be configured by the macro
    u32 key;
} HASHSET_ENTRY;
#pragma pack(pop)

typedef struct HASHSET {
    HASHSET_ENTRY* entries;
    u32 size;
    u32 capacity;
    u32 tombstones;
} HASHSET;

u8 HASHSET_init(HASHSET* hashset, u32 capacity);
HASHSET* HASHSET_create(u32 capacity);

void HASHSET_deinit(HASHSET* hashset);
void HASHSET_destroy(HASHSET* hashset);

u8 HASHSET_grow(HASHSET* hashset);
u8 HASHSET_add(HASHSET* hashset, u32 key);
u8 HASHSET_quick_add(HASHSET* hashset, u32 hash, u32 key);
void HASHSET_remove(HASHSET* hashset, u32 key);

u32 HASHSET_hash(const u8* data, u32 size);

u8 HASHSET_contains(const HASHSET* hashset, u32 key);
HASHSET_ENTRY* HASHSET_find(const HASHSET* hashset, u32 key);

HASHSET* HASHSET_union(const HASHSET* a, const HASHSET* b);
HASHSET* HASHSET_intersection(const HASHSET* a, const HASHSET* b);
HASHSET* HASHSET_difference(const HASHSET* a, const HASHSET* b);

#endif // NESQUIK_HASHSET_H