#include <string.h>
#include <stdlib.h>

#include "hash/hash.h"
#include "hash/pointer_hashtable.h"

POINTER_HASHTABLE_DECLARE(u64, u64)

u8 POINTER_HASHTABLE_u64_u64_init(POINTER_HASHTABLE_u64_u64* hashtable,
                  const u32 capacity,
                  u32 (*key_size)(const u64*),
                  u8 (*key_equal)(const u64*, const u64*)) {

    if (hashtable == NULL) return 0;

    hashtable->size = 0;
    hashtable->capacity = capacity < POINTER_HASHTABLE_MIN_CAPACITY ? POINTER_HASHTABLE_MIN_CAPACITY : capacity;
    hashtable->tombstones = 0;

    hashtable->key_size = key_size;
    hashtable->key_equal = key_equal;

    hashtable->entries = (POINTER_HASHTABLE_ENTRY_u64_u64*)malloc(sizeof(POINTER_HASHTABLE_ENTRY_u64_u64) * capacity);
    if (hashtable->entries == NULL) {
        hashtable->capacity = 0;
        return 0;
    }

    memset(hashtable->entries, 0, sizeof(POINTER_HASHTABLE_ENTRY_u64_u64) * capacity);

    return 1;
}

POINTER_HASHTABLE_u64_u64* POINTER_HASHTABLE_u64_u64_create(const u32 capacity,
    u32 (*key_size)(const u64*),
    u8 (*key_equal)(const u64*, const u64*)) {

    POINTER_HASHTABLE_u64_u64* hashtable = (POINTER_HASHTABLE_u64_u64*)malloc(sizeof(POINTER_HASHTABLE_u64_u64));
    if (hashtable == NULL) return NULL;

    const u8 r = POINTER_HASHTABLE_u64_u64_init(hashtable, capacity, key_size, key_equal);
    if (r == 0) {
        free(hashtable);
        return NULL;
    }

    return hashtable;
}

void POINTER_HASHTABLE_u64_u64_deinit(POINTER_HASHTABLE_u64_u64* hashtable) {
    if (hashtable == NULL) return;
    hashtable->size = 0;
    hashtable->capacity = 0;
    hashtable->tombstones = 0;

    hashtable->key_size = NULL;
    hashtable->key_equal = NULL;

    if (hashtable->entries != NULL) {
        free(hashtable->entries);
        hashtable->entries = NULL;
    }
}

void POINTER_HASHTABLE_u64_u64_destroy(POINTER_HASHTABLE_u64_u64* hashtable) {
    if (hashtable == NULL) return;

    POINTER_HASHTABLE_u64_u64_deinit(hashtable);
    free(hashtable);
}

u8 POINTER_HASHTABLE_u64_u64_grow(POINTER_HASHTABLE_u64_u64* hashtable) {
    if (hashtable == NULL) return 0;

    u32 new_capacity = (u32)(hashtable->capacity * POINTER_HASHTABLE_MAX_LOAD_FACTOR / POINTER_HASHTABLE_MIN_LOAD_FACTOR);
    new_capacity = (new_capacity > hashtable->capacity) ? new_capacity : hashtable->capacity;

    POINTER_HASHTABLE_u64_u64 new_hashtable;
    u8 r = POINTER_HASHTABLE_u64_u64_init(&new_hashtable, new_capacity, hashtable->key_size, hashtable->key_equal);
    if (r == 0) return 0;

    for (u32 i = 0; i < hashtable->capacity; i++) {
        const POINTER_HASHTABLE_ENTRY_u64_u64* entry = hashtable->entries + i;
        const u8 status = entry->status;

        if (status == POINTER_HASHTABLE_ENTRY_STATUS_EMPTY || status == POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE) continue;
        r = POINTER_HASHTABLE_u64_u64_quick_add(&new_hashtable, entry->hash, entry->key, entry->value);
        if (r == 0) {
            POINTER_HASHTABLE_u64_u64_deinit(&new_hashtable);
            return 0;
        }
    }

    free(hashtable->entries);
    hashtable->entries = new_hashtable.entries;
    hashtable->capacity = new_capacity;
    hashtable->tombstones = 0;

    return 1;
}

u8 POINTER_HASHTABLE_u64_u64_quick_add(POINTER_HASHTABLE_u64_u64* hashtable, const u32 hash, u64* key, u64 value) {
    if (hashtable == NULL) return 0;

    if ((hashtable->size + 0.0) / hashtable->capacity >= POINTER_HASHTABLE_MAX_LOAD_FACTOR) {
        const u8 r = POINTER_HASHTABLE_u64_u64_grow(hashtable);
        if (r == 0) return 0;
    }

    u32 i = hash % hashtable->capacity;

    POINTER_HASHTABLE_ENTRY_u64_u64* found_entry = NULL;
    do {
        POINTER_HASHTABLE_ENTRY_u64_u64* entry = hashtable->entries + i;
        const u8 status = entry->status;

        if (status == POINTER_HASHTABLE_ENTRY_STATUS_EMPTY || status == POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE) {
            found_entry = entry;
            break;
        }

        const u8 equal = hashtable->key_equal(entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashtable->capacity;
    } while (i != hash % hashtable->capacity);

    if (found_entry == NULL) return 0;

    found_entry->status = POINTER_HASHTABLE_ENTRY_STATUS_FILLED;
    found_entry->hash = hash;
    found_entry->key = key;
    found_entry->value = value;

    hashtable->size++;
    return 1;
}

u8 POINTER_HASHTABLE_u64_u64_add(POINTER_HASHTABLE_u64_u64* hashtable, u64* key, u64 value) {
    if (hashtable == NULL) return 0;

    const u32 key_size = hashtable->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    return POINTER_HASHTABLE_u64_u64_quick_add(hashtable, hash, key, value);
}

void POINTER_HASHTABLE_remove(POINTER_HASHTABLE_u64_u64* hashtable, const u64* key) {
    if (hashtable == NULL) return;

    POINTER_HASHTABLE_ENTRY_u64_u64* entry = POINTER_HASHTABLE_u64_u64_find(hashtable, key);
    if (entry == NULL) return;

    memset(entry, 0, sizeof(POINTER_HASHTABLE_ENTRY_u64_u64));
    entry->status = POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE;
    hashtable->size--;
    hashtable->tombstones++;
}

u32 POINTER_HASHTABLE_u64_u64_hash(const u8* data, const u32 size) {
    return HASH_fnv1a(data, size);
}

u8 POINTER_HASHTABLE_u64_u64_contains(const POINTER_HASHTABLE_u64_u64* hashtable, const u64* key) {
    if (hashtable == NULL) return 0;

    const POINTER_HASHTABLE_ENTRY_u64_u64* entry = POINTER_HASHTABLE_u64_u64_find(hashtable, key);
    if (entry == NULL) return 0;
    return 1;
}

POINTER_HASHTABLE_ENTRY_u64_u64* POINTER_HASHTABLE_u64_u64_find(const POINTER_HASHTABLE_u64_u64* hashtable, const u64* key) {
    if (hashtable == NULL) return NULL;

    // Determine the key_size
    const u32 key_size = hashtable->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    u32 i = hash % hashtable->capacity;

    POINTER_HASHTABLE_ENTRY_u64_u64* entry;
    u8 status;

    do {
        entry = hashtable->entries + i;
        status = entry->status;

        if (status == POINTER_HASHTABLE_ENTRY_STATUS_EMPTY) return NULL;
        if (status == POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {
            i = (i + 1) % hashtable->capacity;
            continue;
        }

        const u8 equal = hashtable->key_equal(entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashtable->capacity;
    } while (i != hash % hashtable->capacity);

    if (status == POINTER_HASHTABLE_ENTRY_STATUS_FILLED) return entry;
    return NULL;
}