#include <string.h>
#include <stdlib.h>

#include "hash/hash.h"
#include "hash/pointer_hashset.h"

u8 POINTER_HASHSET_init(POINTER_HASHSET* hashset,
                  const u32 capacity,
                  u32 (*key_size)(const char*),
                  u8 (*key_equal)(const char*, const char*)) {

    if (hashset == NULL) return 0;

    hashset->size = 0;
    hashset->capacity = capacity < POINTER_HASHSET_MIN_CAPACITY ? POINTER_HASHSET_MIN_CAPACITY : capacity;
    hashset->tombstones = 0;

    hashset->key_size = key_size;
    hashset->key_equal = key_equal;

    hashset->entries = (POINTER_HASHSET_ENTRY*)malloc(sizeof(POINTER_HASHSET_ENTRY) * capacity);
    if (hashset->entries == NULL) {
        hashset->capacity = 0;
        return 0;
    }

    memset(hashset->entries, 0, sizeof(POINTER_HASHSET_ENTRY) * capacity);

    return 1;
}

POINTER_HASHSET* POINTER_HASHSET_create(const u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char*, const char*)) {

    POINTER_HASHSET* hashset = (POINTER_HASHSET*)malloc(sizeof(POINTER_HASHSET));
    if (hashset == NULL) return NULL;

    const u8 r = POINTER_HASHSET_init(hashset, capacity, key_size, key_equal);
    if (r == 0) {
        free(hashset);
        return NULL;
    }

    return hashset;
}

void POINTER_HASHSET_deinit(POINTER_HASHSET* hashset) {
    if (hashset == NULL) return;
    hashset->size = 0;
    hashset->capacity = 0;
    hashset->tombstones = 0;

    hashset->key_size = NULL;
    hashset->key_equal = NULL;

    if (hashset->entries != NULL) {
        free(hashset->entries);
        hashset->entries = NULL;
    }
}

void POINTER_HASHSET_destroy(POINTER_HASHSET* hashset) {
    if (hashset == NULL) return;

    POINTER_HASHSET_deinit(hashset);
    free(hashset);
}

u8 POINTER_HASHSET_grow(POINTER_HASHSET* hashset) {
    if (hashset == NULL) return 0;

    u32 new_capacity = (u32)(hashset->capacity * POINTER_HASHSET_MAX_LOAD_FACTOR / POINTER_HASHSET_MIN_LOAD_FACTOR);
    new_capacity = (new_capacity > hashset->capacity) ? new_capacity : hashset->capacity;

    POINTER_HASHSET new_hashset;
    u8 r = POINTER_HASHSET_init(&new_hashset, new_capacity, hashset->key_size, hashset->key_equal);
    if (r == 0) return 0;

    for (u32 i = 0; i < hashset->capacity; i++) {
        const POINTER_HASHSET_ENTRY* entry = hashset->entries + i;
        const u8 status = entry->status;

        if (status == POINTER_HASHSET_ENTRY_STATUS_EMPTY || status == POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE) continue;
        r = POINTER_HASHSET_quick_add(&new_hashset, entry->hash, entry->key);
        if (r == 0) {
            POINTER_HASHSET_deinit(&new_hashset);
            return 0;
        }
    }

    free(hashset->entries);
    hashset->entries = new_hashset.entries;
    hashset->capacity = new_capacity;
    hashset->tombstones = 0;

    return 1;
}

u8 POINTER_HASHSET_quick_add(POINTER_HASHSET* hashset, const u32 hash, char* key) {
    if (hashset == NULL) return 0;

    if ((hashset->size + 0.0) / hashset->capacity >= POINTER_HASHSET_MAX_LOAD_FACTOR) {
        const u8 r = POINTER_HASHSET_grow(hashset);
        if (r == 0) return 0;
    }

    u32 i = hash % hashset->capacity;

    POINTER_HASHSET_ENTRY* found_entry = NULL;
    do {
        POINTER_HASHSET_ENTRY* entry = hashset->entries + i;
        const u8 status = entry->status;

        if (status == POINTER_HASHSET_ENTRY_STATUS_EMPTY || status == POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE) {
            found_entry = entry;
            break;
        }

        const u8 equal = hashset->key_equal(entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashset->capacity;
    } while (i != hash % hashset->capacity);

    if (found_entry == NULL) return 0;

    found_entry->status = POINTER_HASHSET_ENTRY_STATUS_FILLED;
    found_entry->hash = hash;
    found_entry->key = key;

    hashset->size++;
    return 1;
}

u8 POINTER_HASHSET_add(POINTER_HASHSET* hashset, char* key) {
    if (hashset == NULL) return 0;

    const u32 key_size = hashset->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    return POINTER_HASHSET_quick_add(hashset, hash, key);
}

void POINTER_HASHSET_remove(POINTER_HASHSET* hashset, const char* key) {
    if (hashset == NULL) return;

    POINTER_HASHSET_ENTRY* entry = POINTER_HASHSET_find(hashset, key);
    if (entry == NULL) return;

    memset(entry, 0, sizeof(POINTER_HASHSET_ENTRY));
    entry->status = POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE;
    hashset->size--;
    hashset->tombstones++;
}

u32 POINTER_HASHSET_hash(const u8* data, const u32 size) {
    return HASH_fnv1a(data, size);
}

u8 POINTER_HASHSET_contains(const POINTER_HASHSET* hashset, const char* key) {
    if (hashset == NULL) return 0;

    const POINTER_HASHSET_ENTRY* entry = POINTER_HASHSET_find(hashset, key);
    if (entry == NULL) return 0;
    return 1;
}

POINTER_HASHSET_ENTRY* POINTER_HASHSET_find(const POINTER_HASHSET* hashset, const char* key) {
    if (hashset == NULL) return NULL;

    // Determine the key_size
    const u32 key_size = hashset->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    u32 i = hash % hashset->capacity;

    POINTER_HASHSET_ENTRY* entry;
    u8 status;

    do {
        entry = hashset->entries + i;
        status = entry->status;

        if (status == POINTER_HASHSET_ENTRY_STATUS_EMPTY) return NULL;
        if (status == POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {
            i = (i + 1) % hashset->capacity;
            continue;
        }

        const u8 equal = hashset->key_equal(entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashset->capacity;
    } while (i != hash % hashset->capacity);

    if (status == POINTER_HASHSET_ENTRY_STATUS_FILLED) return entry;
    return NULL;
}