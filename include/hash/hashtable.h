#ifndef NESQUIK_HASHTABLE_H
#define NESQUIK_HASHTABLE_H

#include <stdlib.h>

#include "types.h"

#define HASHTABLE_ENTRY_STATUS_EMPTY      0
#define HASHTABLE_ENTRY_STATUS_FILLED     1
#define HASHTABLE_ENTRY_STATUS_TOMBSTONE  2

#define HASHTABLE_MIN_LOAD_FACTOR 0.5
#define HASHTABLE_MAX_LOAD_FACTOR 0.75

#define HASHTABLE_MIN_CAPACITY 8

#pragma pack(push, 1)
typedef struct HASHTABLE_ENTRY {
    u8 status;
    u32 hash;
    // These need to be configured by the macro
    char* key;
    char* value;
} HASHTABLE_ENTRY;
#pragma pack(pop)

typedef struct HASHTABLE {
    HASHTABLE_ENTRY* entries;
    u32 size;
    u32 capacity;
    u32 tombstones;

    u32 (*key_size)(const char*);
    u8 (*key_equal)(const char*, const char*);
} HASHTABLE;

u8 HASHTABLE_init(HASHTABLE* hashtable,
    u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char* a, const char* b));
HASHTABLE* HASHTABLE_create(u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char* a, const char* b));

void HASHTABLE_deinit(HASHTABLE* hashtable);
void HASHTABLE_destroy(HASHTABLE* hashtable);

u8 HASHTABLE_grow(HASHTABLE* hashtable);
u8 HASHTABLE_add(HASHTABLE* hashtable, char* key, char* value);
u8 HASHTABLE_quick_add(HASHTABLE* hashtable, u32 hash, char* key, char* value);
void HASHTABLE_remove(HASHTABLE* hashtable, const char* key);

u32 HASHTABLE_hash(const u8* data, u32 size);

u8 HASHTABLE_contains(const HASHTABLE* hashtable, const char* key);
HASHTABLE_ENTRY* HASHTABLE_find(const HASHTABLE* hashtable, const char* key);

#endif //NESQUIK_HASHTABLE_H