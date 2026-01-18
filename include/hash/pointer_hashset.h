#ifndef NESQUIK_POINTER_HASHSET_H
#define NESQUIK_POINTER_HASHSET_H

#include "types.h"

#define POINTER_HASHSET_ENTRY_STATUS_EMPTY      0
#define POINTER_HASHSET_ENTRY_STATUS_FILLED     1
#define POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE  2

#define POINTER_HASHSET_MIN_LOAD_FACTOR         0.5
#define POINTER_HASHSET_MAX_LOAD_FACTOR         0.75

#define POINTER_HASHSET_MIN_CAPACITY            8

#pragma pack(push, 1)
typedef struct POINTER_HASHSET_ENTRY {
    u8 status;
    u32 hash;
    // These need to be configured by the macro
    char* key;
} POINTER_HASHSET_ENTRY;
#pragma pack(pop)

typedef struct POINTER_HASHSET {
    POINTER_HASHSET_ENTRY* entries;
    u32 size;
    u32 capacity;
    u32 tombstones;

    u32 (*key_size)(const char*);
    u8 (*key_equal)(const char*, const char*);
} POINTER_HASHSET;

u8 POINTER_HASHSET_init(POINTER_HASHSET* hashset,
    u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char*, const char*));
POINTER_HASHSET* POINTER_HASHSET_create(u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char*, const char*));

void POINTER_HASHSET_deinit(POINTER_HASHSET* hashset);
void POINTER_HASHSET_destroy(POINTER_HASHSET* hashset);

u8 POINTER_HASHSET_grow(POINTER_HASHSET* hashset);
u8 POINTER_HASHSET_add(POINTER_HASHSET* hashset, char* key);
u8 POINTER_HASHSET_quick_add(POINTER_HASHSET* hashset, u32 hash, char* key);
void POINTER_HASHSET_remove(POINTER_HASHSET* hashset, const char* key);

u32 POINTER_HASHSET_hash(const u8* data, u32 size);

u8 POINTER_HASHSET_contains(const POINTER_HASHSET* hashset, const char* key);
POINTER_HASHSET_ENTRY* POINTER_HASHSET_find(const POINTER_HASHSET* hashset, const char* key);

POINTER_HASHSET* POINTER_HASHSET_union(const POINTER_HASHSET* a, const POINTER_HASHSET* b);
POINTER_HASHSET* POINTER_HASHSET_intersection(const POINTER_HASHSET* a, const POINTER_HASHSET* b);
POINTER_HASHSET* POINTER_HASHSET_difference(const POINTER_HASHSET* a, const POINTER_HASHSET* b);

#endif // NESQUIK_POINTER_HASHSET_H