#include <string.h>
#include <stdlib.h>

#include "hash/hash.h"
#include "hash/hashtable.h"

HASHTABLE_DECLARE(u64, u64)

u8 HASHTABLE_u64_u64_init(HASHTABLE_u64_u64* hashtable, const u32 capacity) {

    if (hashtable == NULL) return 0;

    hashtable->size = 0;
    hashtable->capacity = capacity < HASHTABLE_MIN_CAPACITY ? HASHTABLE_MIN_CAPACITY : capacity;
    hashtable->tombstones = 0;

    hashtable->entries = (HASHTABLE_ENTRY_u64_u64*)malloc(sizeof(HASHTABLE_ENTRY_u64_u64) * capacity);
    if (hashtable->entries == NULL) {
        hashtable->capacity = 0;
        return 0;
    }

    memset(hashtable->entries, 0, sizeof(HASHTABLE_ENTRY_u64_u64) * capacity);

    return 1;
}

HASHTABLE_u64_u64* HASHTABLE_create(const u32 capacity) {

    HASHTABLE_u64_u64* hashtable = (HASHTABLE_u64_u64*)malloc(sizeof(HASHTABLE_u64_u64));
    if (hashtable == NULL) return NULL;

    const u8 r = HASHTABLE_u64_u64_init(hashtable, capacity);
    if (r == 0) {
        free(hashtable);
        return NULL;
    }

    return hashtable;
}

void HASHTABLE_u64_u64_deinit(HASHTABLE_u64_u64* hashtable) {
    if (hashtable == NULL) return;
    hashtable->size = 0;
    hashtable->capacity = 0;
    hashtable->tombstones = 0;

    if (hashtable->entries != NULL) {
        free(hashtable->entries);
        hashtable->entries = NULL;
    }
}

void HASHTABLE_u64_u64_destroy(HASHTABLE_u64_u64* hashtable) {
    if (hashtable == NULL) return;

    HASHTABLE_u64_u64_deinit(hashtable);
    free(hashtable);
}

u8 HASHTABLE_u64_u64_grow(HASHTABLE_u64_u64* hashtable) {
    if (hashtable == NULL) return 0;

    u32 new_capacity = (u32)(hashtable->capacity * HASHTABLE_MAX_LOAD_FACTOR / HASHTABLE_MIN_LOAD_FACTOR);
    new_capacity = (new_capacity > hashtable->capacity) ? new_capacity : hashtable->capacity;

    HASHTABLE_u64_u64 new_hashtable;
    u8 r = HASHTABLE_u64_u64_init(&new_hashtable, new_capacity);
    if (r == 0) return 0;

    for (u32 i = 0; i < hashtable->capacity; i++) {
        const HASHTABLE_ENTRY_u64_u64* entry = hashtable->entries + i;
        const u8 status = entry->status;

        if (status == HASHTABLE_ENTRY_STATUS_EMPTY || status == HASHTABLE_ENTRY_STATUS_TOMBSTONE) continue;
        r = HASHTABLE_u64_u64_quick_add(&new_hashtable, entry->hash, entry->key, entry->value);
        if (r == 0) {
            HASHTABLE_u64_u64_deinit(&new_hashtable);
            return 0;
        }
    }

    free(hashtable->entries);
    hashtable->entries = new_hashtable.entries;
    hashtable->capacity = new_capacity;
    hashtable->tombstones = 0;

    return 1;
}

u8 HASHTABLE_u64_u64_quick_add(HASHTABLE_u64_u64* hashtable, const u32 hash, const u64 key, const u64 value) {
    if (hashtable == NULL) return 0;

    if ((hashtable->size + 0.0) / hashtable->capacity >= HASHTABLE_MAX_LOAD_FACTOR) {
        const u8 r = HASHTABLE_u64_u64_grow(hashtable);
        if (r == 0) return 0;
    }

    u32 i = hash % hashtable->capacity;

    HASHTABLE_ENTRY_u64_u64* found_entry = NULL;
    do {
        HASHTABLE_ENTRY_u64_u64* entry = hashtable->entries + i;
        const u8 status = entry->status;

        if (status == HASHTABLE_ENTRY_STATUS_EMPTY || status == HASHTABLE_ENTRY_STATUS_TOMBSTONE) {
            found_entry = entry;
            break;
        }

        if (entry->key == key) break;
        i = (i + 1) % hashtable->capacity;
    } while (i != hash % hashtable->capacity);

    if (found_entry == NULL) return 0;

    found_entry->status = HASHTABLE_ENTRY_STATUS_FILLED;
    found_entry->hash = hash;
    found_entry->key = key;
    found_entry->value = value;

    hashtable->size++;
    return 1;
}

u8 HASHTABLE_u64_u64_add(HASHTABLE_u64_u64* hashtable, const u64 key, const u64 value) {
    if (hashtable == NULL) return 0;

    const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));
    return HASHTABLE_u64_u64_quick_add(hashtable, hash, key, value);
}

void HASHTABLE_u64_u64_remove(HASHTABLE_u64_u64* hashtable, const u64 key) {
    if (hashtable == NULL) return;

    HASHTABLE_ENTRY_u64_u64* entry = HASHTABLE_u64_u64_find(hashtable, key);
    if (entry == NULL) return;

    memset(entry, 0, sizeof(HASHTABLE_ENTRY_u64_u64));
    entry->status = HASHTABLE_ENTRY_STATUS_TOMBSTONE;
    hashtable->size--;
    hashtable->tombstones++;
}

u32 HASHTABLE_u64_u64_hash(const u8* data, const u32 size) {
    return HASH_fnv1a(data, size);
}

u8 HASHTABLE_u64_u64_contains(const HASHTABLE_u64_u64* hashtable, const u64 key) {
    if (hashtable == NULL) return 0;

    const HASHTABLE_ENTRY_u64_u64* entry = HASHTABLE_u64_u64_find(hashtable, key);
    if (entry == NULL) return 0;
    return 1;
}

HASHTABLE_ENTRY_u64_u64* HASHTABLE_u64_u64_find(const HASHTABLE_u64_u64* hashtable, const u64 key) {
    if (hashtable == NULL) return NULL;

    // Determine the key_size
    const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));
    u32 i = hash % hashtable->capacity;

    HASHTABLE_ENTRY_u64_u64* entry;
    u8 status;

    do {
        entry = hashtable->entries + i;
        status = entry->status;

        if (status == HASHTABLE_ENTRY_STATUS_EMPTY) return NULL;
        if (status == HASHTABLE_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {
            i = (i + 1) % hashtable->capacity;
            continue;
        }

        if (entry->key == key) break;
        i = (i + 1) % hashtable->capacity;
    } while (i != hash % hashtable->capacity);

    if (status == HASHTABLE_ENTRY_STATUS_FILLED) return entry;
    return NULL;
}