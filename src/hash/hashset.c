#include <string.h>
#include <stdlib.h>

#include "hash/hash.h"
#include "hash/hashset.h"

HASHSET_DECLARE(u32)

u8 HASHSET_u32_init(HASHSET_u32* hashset, const u32 capacity) {
    if (hashset == NULL) return 0;

    hashset->size = 0;
    hashset->capacity = capacity < HASHSET_MIN_CAPACITY ? HASHSET_MIN_CAPACITY : capacity;
    hashset->tombstones = 0;

    hashset->entries = (HASHSET_ENTRY_u32*)malloc(sizeof(HASHSET_ENTRY_u32) * capacity);
    if (hashset->entries == NULL) {
        hashset->capacity = 0;
        return 0;
    }

    memset(hashset->entries, 0, sizeof(HASHSET_ENTRY_u32) * capacity);

    return 1;
}

HASHSET_u32* HASHSET_u32_create(const u32 capacity) {
    HASHSET_u32* hashset = (HASHSET_u32*)malloc(sizeof(HASHSET_u32));
    if (hashset == NULL) return NULL;

    const u8 r = HASHSET_u32_init(hashset, capacity);
    if (r == 0) {
        free(hashset);
        return NULL;
    }

    return hashset;
}

void HASHSET_u32_deinit(HASHSET_u32* hashset) {
    if (hashset == NULL) return;
    hashset->size = 0;
    hashset->capacity = 0;
    hashset->tombstones = 0;

    if (hashset->entries != NULL) {
        free(hashset->entries);
        hashset->entries = NULL;
    }
}

void HASHSET_u32_destroy(HASHSET_u32* hashset) {
    if (hashset == NULL) return;

    HASHSET_u32_deinit(hashset);
    free(hashset);
}

u8 HASHSET_u32_grow(HASHSET_u32* hashset) {
    if (hashset == NULL) return 0;

    u32 new_capacity = (u32)(hashset->capacity * HASHSET_MAX_LOAD_FACTOR / HASHSET_MIN_LOAD_FACTOR);
    new_capacity = (new_capacity > hashset->capacity) ? new_capacity : hashset->capacity;

    HASHSET_u32 new_hashset;
    u8 r = HASHSET_u32_init(&new_hashset, new_capacity);
    if (r == 0) return 0;

    for (u32 i = 0; i < hashset->capacity; i++) {
        const HASHSET_ENTRY_u32* entry = hashset->entries + i;
        const u8 status = entry->status;

        if (status == HASHSET_ENTRY_STATUS_EMPTY || status == HASHSET_ENTRY_STATUS_TOMBSTONE) continue;
        r = HASHSET_u32_quick_add(&new_hashset, entry->hash, entry->key);
        if (r == 0) {
            HASHSET_u32_deinit(&new_hashset);
            return 0;
        }
    }

    free(hashset->entries);
    hashset->entries = new_hashset.entries;
    hashset->capacity = new_capacity;
    hashset->tombstones = 0;

    return 1;
}

u8 HASHSET_u32_quick_add(HASHSET_u32* hashset, const u32 hash, const u32 key) {
    if (hashset == NULL) return 0;

    if ((hashset->size + 0.0) / hashset->capacity >= HASHSET_MAX_LOAD_FACTOR) {
        const u8 r = HASHSET_u32_grow(hashset);
        if (r == 0) return 0;
    }

    u32 i = hash % hashset->capacity;

    HASHSET_ENTRY_u32* found_entry = NULL;
    do {
        HASHSET_ENTRY_u32* entry = hashset->entries + i;
        const u8 status = entry->status;

        if (status == HASHSET_ENTRY_STATUS_EMPTY || status == HASHSET_ENTRY_STATUS_TOMBSTONE) {
            found_entry = entry;
            break;
        }

        if (entry->key == key) break;
        i = (i + 1) % hashset->capacity;
    } while (i != hash % hashset->capacity);

    if (found_entry == NULL) return 0;

    found_entry->status = HASHSET_ENTRY_STATUS_FILLED;
    found_entry->hash = hash;
    found_entry->key = key;

    hashset->size++;
    return 1;
}

u8 HASHSET_u32_add(HASHSET_u32* hashset, const u32 key) {
    if (hashset == NULL) return 0;

    const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));
    return HASHSET_u32_quick_add(hashset, hash, key);
}

void HASHSET_u32_remove(HASHSET_u32* hashset, const u32 key) {
    if (hashset == NULL) return;

    HASHSET_ENTRY_u32* entry = HASHSET_u32_find(hashset, key);
    if (entry == NULL) return;

    memset(entry, 0, sizeof(HASHSET_ENTRY_u32));
    entry->status = HASHSET_ENTRY_STATUS_TOMBSTONE;
    hashset->size--;
    hashset->tombstones++;
}

u32 HASHSET_u32_hash(const u8* data, const u32 size) {
    return HASH_fnv1a(data, size);
}

u8 HASHSET_u32_contains(const HASHSET_u32* hashset, const u32 key) {
    if (hashset == NULL) return 0;

    const HASHSET_ENTRY_u32* entry = HASHSET_u32_find(hashset, key);
    if (entry == NULL) return 0;
    return 1;
}

HASHSET_ENTRY_u32* HASHSET_u32_find(const HASHSET_u32* hashset, const u32 key) {
    if (hashset == NULL) return NULL;

    // Determine the key_size
    const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));
    u32 i = hash % hashset->capacity;

    HASHSET_ENTRY_u32* entry;
    u8 status;

    do {
        entry = hashset->entries + i;
        status = entry->status;

        if (status == HASHSET_ENTRY_STATUS_EMPTY) return NULL;
        if (status == HASHSET_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {
            i = (i + 1) % hashset->capacity;
            continue;
        }

        if (entry->key == key) break;
        i = (i + 1) % hashset->capacity;
    } while (i != hash % hashset->capacity);

    if (status == HASHSET_ENTRY_STATUS_FILLED) return entry;
    return NULL;
}

HASHSET_u32* HASHSET_u32_union(const HASHSET_u32* a, const HASHSET_u32* b) {
    if (a == NULL || b == NULL) return NULL;

    HASHSET_u32* c = HASHSET_u32_create(a->capacity + b->capacity);
    if (c == NULL) return NULL;

    for (u32 ai = 0; ai < a->capacity; ai++) {
        const HASHSET_ENTRY_u32 entry = a->entries[ai];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED)
            HASHSET_u32_quick_add(c, entry.hash, entry.key);
    }

    for (u32 bi = 0; bi < b->capacity; bi++) {
        const HASHSET_ENTRY_u32 entry = b->entries[bi];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED)
            HASHSET_u32_quick_add(c, entry.hash, entry.key);
    }

    return c;
}

HASHSET_u32* HASHSET_u32_intersection(const HASHSET_u32* a, const HASHSET_u32* b) {
    if (a == NULL || b == NULL) return NULL;

    const HASHSET_u32* smaller;
    const HASHSET_u32* larger;
    if (a->capacity < b->capacity) {
        smaller = a;
        larger = b;
    }
    else {
        smaller = b;
        larger = a;
    }

    HASHSET_u32* c = HASHSET_u32_create(smaller->capacity);
    if (c == NULL) return NULL;

    for (u32 i = 0; i < smaller->capacity; i++) {
        const HASHSET_ENTRY_u32 entry = smaller->entries[i];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED) {
            if (HASHSET_u32_contains(larger, entry.key) == 1) {
                HASHSET_u32_quick_add(c, entry.hash, entry.key);
            }
        }
    }

    return c;
}

HASHSET_u32* HASHSET_difference(const HASHSET_u32* a, const HASHSET_u32* b) {
    if (a == NULL || b == NULL) return NULL;

    HASHSET_u32* c = HASHSET_u32_create(a->capacity);
    if (c == NULL) return NULL;

    for (u32 ai = 0; ai < a->capacity; ai++) {
        const HASHSET_ENTRY_u32 entry = a->entries[ai];
        if (entry.status == HASHSET_ENTRY_STATUS_FILLED) {
            if (HASHSET_u32_contains(b, entry.key) == 0) {
                HASHSET_u32_quick_add(c, entry.hash, entry.key);
            }
        }
    }

    return c;
}