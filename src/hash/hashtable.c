#include <string.h>

#include "hash/hash.h"
#include "hash/hashtable.h"

u8 HASHTABLE_init(HASHTABLE* hashtable,
                const u32 capacity,
                u32 (*key_size)(const char*),
                u8 (*key_equal)(const char*, const char*)) {

    if (hashtable == NULL) return 0;

    hashtable->size = 0;
    hashtable->capacity = capacity < HASHTABLE_MIN_CAPACITY ? HASHTABLE_MIN_CAPACITY : capacity;
    hashtable->tombstones = 0;

    hashtable->key_size = key_size;
    hashtable->key_equal = key_equal;

    hashtable->entries = (HASHTABLE_ENTRY*)malloc(sizeof(HASHTABLE_ENTRY) * capacity);
    if (hashtable->entries == NULL) {
        hashtable->capacity = 0;
        return 0;
    }

    memset(hashtable->entries, 0, sizeof(HASHTABLE_ENTRY) * capacity);

    return 1;
}

HASHTABLE* HASHTABLE_create(const u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char* a, const char* b)) {

    HASHTABLE* hashtable = (HASHTABLE*)malloc(sizeof(HASHTABLE));
    if (hashtable == NULL) return NULL;

    const u8 r = HASHTABLE_init(hashtable, capacity, key_size, key_equal);
    if (r == 0) {
        free(hashtable);
        return NULL;
    }

    return hashtable;
}

void HASHTABLE_deinit(HASHTABLE* hashtable) {
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

void HASHTABLE_destroy(HASHTABLE* hashtable) {
    if (hashtable == NULL) return;

    HASHTABLE_deinit(hashtable);
    free(hashtable);
}

u8 HASHTABLE_grow(HASHTABLE* hashtable) {
    if (hashtable == NULL) return 0;

    u32 new_capacity = (u32)(hashtable->capacity * HASHTABLE_MAX_LOAD_FACTOR / HASHTABLE_MIN_LOAD_FACTOR);
    new_capacity = (new_capacity > hashtable->capacity) ? new_capacity : hashtable->capacity;

    HASHTABLE new_hashtable;
    u8 r = HASHTABLE_init(&new_hashtable, new_capacity, hashtable->key_size, hashtable->key_equal);
    if (r == 0) return 0;

    for (u32 i = 0; i < hashtable->capacity; i++) {
        const HASHTABLE_ENTRY* entry = hashtable->entries + i;
        const u8 status = entry->status;

        if (status == HASHTABLE_ENTRY_STATUS_EMPTY || status == HASHTABLE_ENTRY_STATUS_TOMBSTONE) continue;
        r = HASHTABLE_quick_add(&new_hashtable, entry->hash, entry->key, entry->value);
        if (r == 0) {
            HASHTABLE_deinit(&new_hashtable);
            return 0;
        }
    }

    free(hashtable->entries);
    hashtable->entries = new_hashtable.entries;
    hashtable->capacity = new_capacity;
    hashtable->tombstones = 0;

    return 1;
}

u8 HASHTABLE_quick_add(HASHTABLE* hashtable, const u32 hash, char* key, char* value) {
    if (hashtable == NULL) return 0;

    if ((hashtable->size + 0.0) / hashtable->capacity >= HASHTABLE_MAX_LOAD_FACTOR) {
        const u8 r = HASHTABLE_grow(hashtable);
        if (r == 0) return 0;
    }

    u32 i = hash % hashtable->capacity;

    HASHTABLE_ENTRY* found_entry = NULL;
    do {
        HASHTABLE_ENTRY *curr_entry = hashtable->entries + i;
        const u8 status = curr_entry->status;

        if (status == HASHTABLE_ENTRY_STATUS_EMPTY || status == HASHTABLE_ENTRY_STATUS_TOMBSTONE) {
            found_entry = curr_entry;
            break;
        }

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

u8 HASHTABLE_add(HASHTABLE* hashtable, char* key, char* value) {
    if (hashtable == NULL) return 0;

    const u32 key_size = hashtable->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    return HASHTABLE_quick_add(hashtable, hash, key, value);
}

void HASHTABLE_remove(HASHTABLE* hashtable, const char* key) {
    if (hashtable == NULL) return;

    HASHTABLE_ENTRY* entry = HASHTABLE_find(hashtable, key);
    if (entry == NULL) return;

    memset(entry, 0, sizeof(HASHTABLE_ENTRY));
    entry->status = HASHTABLE_ENTRY_STATUS_TOMBSTONE;
    hashtable->size--;
    hashtable->tombstones++;
}

u32 HASHTABLE_hash(const u8* data, const u32 size) {
    return HASH_fnv1a(data, size);
}

u8 HASHTABLE_contains(const HASHTABLE* hashtable, const char* key) {
    if (hashtable == NULL) return 0;

    const HASHTABLE_ENTRY* entry = HASHTABLE_find(hashtable, key);
    if (entry == NULL) return 0;
    return 1;
}

HASHTABLE_ENTRY* HASHTABLE_find(const HASHTABLE* hashtable, const char* key) {
    if (hashtable == NULL) return NULL;

    // Determine the key_size
    const u32 key_size = hashtable->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    u32 i = hash % hashtable->capacity;

    HASHTABLE_ENTRY* entry;
    u8 status;

    do {
        entry = hashtable->entries + i;
        status = entry->status;

        if (status == HASHTABLE_ENTRY_STATUS_EMPTY) return NULL;
        if (status == HASHTABLE_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {
            i = (i + 1) % hashtable->capacity;
            continue;
        }

        const u8 equal = hashtable->key_equal(entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashtable->capacity;
    } while (i != hash % hashtable->capacity);

    if (status == HASHTABLE_ENTRY_STATUS_FILLED) return entry;
    return NULL;
}