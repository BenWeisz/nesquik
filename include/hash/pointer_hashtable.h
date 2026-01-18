#ifndef NESQUIK_POINTER_HASHTABLE_H
#define NESQUIK_POINTER_HASHTABLE_H

#include "types.h"

#define POINTER_HASHTABLE_ENTRY_STATUS_EMPTY        0
#define POINTER_HASHTABLE_ENTRY_STATUS_FILLED       1
#define POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE    2

#define POINTER_HASHTABLE_MIN_LOAD_FACTOR           0.5
#define POINTER_HASHTABLE_MAX_LOAD_FACTOR           0.75

#define POINTER_HASHTABLE_MIN_CAPACITY              8

#pragma pack(push, 1)
typedef struct POINTER_HASHTABLE_ENTRY {
    u8 status;
    u32 hash;
    // These need to be configured by the macro
    char* key;
    char* value;
} POINTER_HASHTABLE_ENTRY;
#pragma pack(pop)

typedef struct POINTER_HASHTABLE {
    POINTER_HASHTABLE_ENTRY* entries;
    u32 size;
    u32 capacity;
    u32 tombstones;

    u32 (*key_size)(const char*);
    u8 (*key_equal)(const char*, const char*);
} POINTER_HASHTABLE;

u8 POINTER_HASHTABLE_init(POINTER_HASHTABLE* hashtable,
    u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char*, const char*));
POINTER_HASHTABLE* POINTER_HASHTABLE_create(u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char*, const char*));

void POINTER_HASHTABLE_deinit(POINTER_HASHTABLE* hashtable);
void POINTER_HASHTABLE_destroy(POINTER_HASHTABLE* hashtable);

u8 POINTER_HASHTABLE_grow(POINTER_HASHTABLE* hashtable);
u8 POINTER_HASHTABLE_add(POINTER_HASHTABLE* hashtable, char* key, char* value);
u8 POINTER_HASHTABLE_quick_add(POINTER_HASHTABLE* hashtable, u32 hash, char* key, char* value);
void POINTER_HASHTABLE_remove(POINTER_HASHTABLE* hashtable, const char* key);

u32 POINTER_HASHTABLE_hash(const u8* data, u32 size);

u8 POINTER_HASHTABLE_contains(const POINTER_HASHTABLE* hashtable, const char* key);
POINTER_HASHTABLE_ENTRY* POINTER_HASHTABLE_find(const POINTER_HASHTABLE* hashtable, const char* key);

#endif // NESQUIK_POINTER_HASHTABLE_H