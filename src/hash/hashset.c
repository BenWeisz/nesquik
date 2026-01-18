#include <string.h>
#include <stdlib.h>

#include "hash/hash.h"
#include "hash/hashset.h"

u8 HASHSET_init(HASHSET* hashset,
                const u32 capacity,
                u32 (*key_size)(const char*),
                u8 (*key_equal)(const char*, const char*)) {

    if (hashset == NULL) return 0;

    hashset->size = 0;
    hashset->capacity = capacity < HASHSET_MIN_CAPACITY ? HASHSET_MIN_CAPACITY : capacity;
    hashset->tombstones = 0;

    hashset->key_size = key_size;
    hashset->key_equal = key_equal;

    hashset->entries = (HASHSET_ENTRY*)malloc(sizeof(HASHSET_ENTRY) * capacity);
    if (hashset->entries == NULL) {
        hashset->capacity = 0;
        return 0;
    }

    memset(hashset->entries, 0, sizeof(HASHSET_ENTRY) * capacity);

    return 1;
}

HASHSET* HASHSET_create(const u32 capacity,
    u32 (*key_size)(const char*),
    u8 (*key_equal)(const char* a, const char* b)) {

    HASHSET* hashset = (HASHSET*)malloc(sizeof(HASHSET));
    if (hashset == NULL) return NULL;

    const u8 r = HASHSET_init(hashset, capacity, key_size, key_equal);
    if (r == 0) {
        free(hashset);
        return NULL;
    }

    return hashset;
}

void HASHSET_deinit(HASHSET* hashset) {
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

void HASHSET_destroy(HASHSET* hashset) {
    if (hashset == NULL) return;

    HASHSET_deinit(hashset);
    free(hashset);
}

u8 HASHSET_grow(HASHSET* hashset) {
    if (hashset == NULL) return 0;

    u32 new_capacity = (u32)(hashset->capacity * HASHSET_MAX_LOAD_FACTOR / HASHSET_MIN_LOAD_FACTOR);
    new_capacity = (new_capacity > hashset->capacity) ? new_capacity : hashset->capacity;

    HASHSET new_hashset;
    u8 r = HASHSET_init(&new_hashset, new_capacity, hashset->key_size, hashset->key_equal);
    if (r == 0) return 0;

    for (u32 i = 0; i < hashset->capacity; i++) {
        const HASHSET_ENTRY* entry = hashset->entries + i;
        const u8 status = entry->status;

        if (status == HASHSET_ENTRY_STATUS_EMPTY || status == HASHSET_ENTRY_STATUS_TOMBSTONE) continue;
        r = HASHSET_quick_add(&new_hashset, entry->hash, entry->key);
        if (r == 0) {
            HASHSET_deinit(&new_hashset);
            return 0;
        }
    }

    free(hashset->entries);
    hashset->entries = new_hashset.entries;
    hashset->capacity = new_capacity;
    hashset->tombstones = 0;

    return 1;
}

u8 HASHSET_quick_add(HASHSET* hashset, const u32 hash, char* key) {
    if (hashset == NULL) return 0;

    if ((hashset->size + 0.0) / hashset->capacity >= HASHSET_MAX_LOAD_FACTOR) {
        const u8 r = HASHSET_grow(hashset);
        if (r == 0) return 0;
    }

    u32 i = hash % hashset->capacity;

    HASHSET_ENTRY* found_entry = NULL;
    do {
        HASHSET_ENTRY* curr_entry = hashset->entries + i;
        const u8 status = curr_entry->status;

        if (status == HASHSET_ENTRY_STATUS_EMPTY || status == HASHSET_ENTRY_STATUS_TOMBSTONE) {
            found_entry = curr_entry;
            break;
        }

        const u8 equal = hashset->key_equal(curr_entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashset->capacity;
    } while (i != hash % hashset->capacity);

    if (found_entry == NULL) return 0;

    found_entry->status = HASHSET_ENTRY_STATUS_FILLED;
    found_entry->hash = hash;
    found_entry->key = key;

    hashset->size++;
    return 1;
}

u8 HASHSET_add(HASHSET* hashset, char* key) {
    if (hashset == NULL) return 0;

    const u32 key_size = hashset->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    return HASHSET_quick_add(hashset, hash, key);
}

void HASHSET_remove(HASHSET* hashset, const char* key) {
    if (hashset == NULL) return;

    HASHSET_ENTRY* entry = HASHSET_find(hashset, key);
    if (entry == NULL) return;

    memset(entry, 0, sizeof(HASHSET_ENTRY));
    entry->status = HASHSET_ENTRY_STATUS_TOMBSTONE;
    hashset->size--;
    hashset->tombstones++;
}

HASHSET* HASHSET_union(const HASHSET* a, const HASHSET* b) {
    if (a == NULL || b == NULL || (a->key_size != b->key_size) || (a->key_equal != b->key_equal)) return NULL;

    HASHSET* c = HASHSET_create(a->capacity + b->capacity, a->key_size, a->key_equal);
    if (c == NULL) return NULL;

    for (u32 ai = 0; ai < a->capacity; ai++) {
        const HASHSET_ENTRY entry = a->entries[ai];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED)
            HASHSET_quick_add(c, entry.hash, entry.key);
    }

    for (u32 bi = 0; bi < b->capacity; bi++) {
        const HASHSET_ENTRY entry = b->entries[bi];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED)
            HASHSET_quick_add(c, entry.hash, entry.key);
    }

    return c;
}

HASHSET* HASHSET_intersection(const HASHSET* a, const HASHSET* b) {
    if (a == NULL || b == NULL || (a->key_size != b->key_size) || (a->key_equal != b->key_equal)) return NULL;

    const HASHSET* smaller;
    const HASHSET* larger;
    if (a->capacity < b->capacity) {
        smaller = a;
        larger = b;
    }
    else {
        smaller = b;
        larger = a;
    }

    HASHSET* c = HASHSET_create(smaller->capacity, smaller->key_size, smaller->key_equal);
    if (c == NULL) return NULL;

    for (u32 i = 0; i < smaller->capacity; i++) {
        const HASHSET_ENTRY entry = smaller->entries[i];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED) {
            if (HASHSET_contains(larger, entry.key) == 1) {
                HASHSET_quick_add(c, entry.hash, entry.key);
            }
        }
    }

    return c;
}

HASHSET* HASHSET_difference(const HASHSET* a, const HASHSET* b) {
    if (a == NULL || b == NULL || (a->key_size != b->key_size) || (a->key_equal != b->key_equal)) return NULL;

    HASHSET* c = HASHSET_create(a->capacity, a->key_size, a->key_equal);
    if (c == NULL) return NULL;

    for (u32 ai = 0; ai < a->capacity; ai++) {
        const HASHSET_ENTRY entry = a->entries[ai];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED) {
            if (HASHSET_contains(b, entry.key) == 0) {
                HASHSET_quick_add(c, entry.hash, entry.key);
            }
        }
    }

    return c;
}

u32 HASHSET_hash(const u8* data, const u32 size) {
    return HASH_fnv1a(data, size);
}

u8 HASHSET_contains(const HASHSET* hashset, const char* key) {
    if (hashset == NULL) return 0;

    const HASHSET_ENTRY* entry = HASHSET_find(hashset, key);
    if (entry == NULL) return 0;
    return 1;
}

HASHSET_ENTRY* HASHSET_find(const HASHSET* hashset, const char* key) {
    if (hashset == NULL) return NULL;

    // Determine the key_size
    const u32 key_size = hashset->key_size(key);
    const u32 hash = HASH_fnv1a((u8*)key, key_size);

    u32 i = hash % hashset->capacity;

    HASHSET_ENTRY* entry;
    u8 status;

    do {
        entry = hashset->entries + i;
        status = entry->status;

        if (status == HASHSET_ENTRY_STATUS_EMPTY) return NULL;
        if (status == HASHSET_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {
            i = (i + 1) % hashset->capacity;
            continue;
        }

        const u8 equal = hashset->key_equal(entry->key, key);
        if (equal == 1) break;
        i = (i + 1) % hashset->capacity;
    } while (i != hash % hashset->capacity);

    if (status == HASHSET_ENTRY_STATUS_FILLED) return entry;
    return NULL;
}