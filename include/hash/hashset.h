#ifndef NESQUIK_HASHSET_H
#define NESQUIK_HASHSET_H

#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "hash/hash.h"

#define HASHSET_ENTRY_STATUS_EMPTY      0
#define HASHSET_ENTRY_STATUS_FILLED     1
#define HASHSET_ENTRY_STATUS_TOMBSTONE  2

#define HASHSET_MIN_LOAD_FACTOR         0.5
#define HASHSET_MAX_LOAD_FACTOR         0.75

#define HASHSET_MIN_CAPACITY            8

#pragma pack(push, 1)
#define HASHSET_DECLARE(K)                                                                                      \
    typedef struct HASHSET_ENTRY_##K {                                                                          \
        u8 status;                                                                                              \
        u32 hash;                                                                                               \
        K key;                                                                                                  \
    } HASHSET_ENTRY_##K;                                                                                        \
                                                                                                                \
    typedef struct HASHSET_##K {                                                                                \
        HASHSET_ENTRY_##K* entries;                                                                             \
        u32 size;                                                                                               \
        u32 capacity;                                                                                           \
        u32 tombstones;                                                                                         \
    } HASHSET_##K;                                                                                              \
                                                                                                                \
    u8 HASHSET_##K##_init(HASHSET_##K* hashset, u32 capacity);                                                  \
    HASHSET_##K* HASHSET_##K##_create(u32 capacity);                                                            \
                                                                                                                \
    void HASHSET_##K##_deinit(HASHSET_##K* hashset);                                                            \
    void HASHSET_##K##_destroy(HASHSET_##K* hashset);                                                           \
                                                                                                                \
    u8 HASHSET_##K##_grow(HASHSET_##K* hashset);                                                                \
    u8 HASHSET_##K##_add(HASHSET_##K* hashset, K key);                                                          \
    u8 HASHSET_##K##_quick_add(HASHSET_##K* hashset, u32 hash, K key);                                          \
    void HASHSET_##K##_remove(HASHSET_##K* hashset, K key);                                                     \
                                                                                                                \
    u32 HASHSET_##K##_hash(const u8* data, u32 size);                                                           \
                                                                                                                \
    u8 HASHSET_##K##_contains(const HASHSET_##K* hashset, K key);                                               \
    HASHSET_ENTRY_##K* HASHSET_##K##_find(const HASHSET_##K* hashset, K key);                                   \
                                                                                                                \
    HASHSET_##K* HASHSET_##K##_union(const HASHSET_##K* a, const HASHSET_##K* b);                               \
    HASHSET_##K* HASHSET_##K##_intersection(const HASHSET_##K* a, const HASHSET_##K* b);                        \
    HASHSET_##K* HASHSET_##K##_difference(const HASHSET_##K* a, const HASHSET_##K* b);
#pragma pack(pop)

#define HASHSET_DEFINE(K)                                                                                       \
    u8 HASHSET_##K##_init(HASHSET_##K* hashset, const u32 capacity) {                                           \
        if (hashset == NULL) return 0;                                                                          \
                                                                                                                \
        hashset->size = 0;                                                                                      \
        hashset->capacity = capacity < HASHSET_MIN_CAPACITY ? HASHSET_MIN_CAPACITY : capacity;                  \
        hashset->tombstones = 0;                                                                                \
                                                                                                                \
        hashset->entries = (HASHSET_ENTRY_##K*)malloc(sizeof(HASHSET_ENTRY_##K) * capacity);                    \
        if (hashset->entries == NULL) {                                                                         \
            hashset->capacity = 0;                                                                              \
            return 0;                                                                                           \
        }                                                                                                       \
                                                                                                                \
        memset(hashset->entries, 0, sizeof(HASHSET_ENTRY_##K) * capacity);                                      \
                                                                                                                \
        return 1;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    HASHSET_##K* HASHSET_##K##_create(const u32 capacity) {                                                     \
        HASHSET_##K* hashset = (HASHSET_##K*)malloc(sizeof(HASHSET_##K));                                       \
        if (hashset == NULL) return NULL;                                                                       \
                                                                                                                \
        const u8 r = HASHSET_##K##_init(hashset, capacity);                                                     \
        if (r == 0) {                                                                                           \
            free(hashset);                                                                                      \
            return NULL;                                                                                        \
        }                                                                                                       \
                                                                                                                \
        return hashset;                                                                                         \
    }                                                                                                           \
                                                                                                                \
    void HASHSET_##K##_deinit(HASHSET_##K* hashset) {                                                           \
        if (hashset == NULL) return;                                                                            \
        hashset->size = 0;                                                                                      \
        hashset->capacity = 0;                                                                                  \
        hashset->tombstones = 0;                                                                                \
                                                                                                                \
        if (hashset->entries != NULL) {                                                                         \
            free(hashset->entries);                                                                             \
            hashset->entries = NULL;                                                                            \
        }                                                                                                       \
    }                                                                                                           \
                                                                                                                \
    void HASHSET_##K##_destroy(HASHSET_##K* hashset) {                                                          \
        if (hashset == NULL) return;                                                                            \
                                                                                                                \
        HASHSET_##K##_deinit(hashset);                                                                          \
        free(hashset);                                                                                          \
    }                                                                                                           \
                                                                                                                \
    u8 HASHSET_##K##_grow(HASHSET_##K* hashset) {                                                               \
        if (hashset == NULL) return 0;                                                                          \
                                                                                                                \
        u32 new_capacity = (u32)(hashset->capacity * HASHSET_MAX_LOAD_FACTOR / HASHSET_MIN_LOAD_FACTOR);        \
        new_capacity = (new_capacity > hashset->capacity) ? new_capacity : hashset->capacity;                   \
                                                                                                                \
        HASHSET_##K new_hashset;                                                                                \
        u8 r = HASHSET_##K##_init(&new_hashset, new_capacity);                                                  \
        if (r == 0) return 0;                                                                                   \
                                                                                                                \
        for (u32 i = 0; i < hashset->capacity; i++) {                                                           \
            const HASHSET_ENTRY_##K* entry = hashset->entries + i;                                              \
            const u8 status = entry->status;                                                                    \
                                                                                                                \
            if (status == HASHSET_ENTRY_STATUS_EMPTY || status == HASHSET_ENTRY_STATUS_TOMBSTONE) continue;     \
            r = HASHSET_##K##_quick_add(&new_hashset, entry->hash, entry->key);                                 \
            if (r == 0) {                                                                                       \
                HASHSET_##K##_deinit(&new_hashset);                                                             \
                return 0;                                                                                       \
            }                                                                                                   \
        }                                                                                                       \
                                                                                                                \
        free(hashset->entries);                                                                                 \
        hashset->entries = new_hashset.entries;                                                                 \
        hashset->capacity = new_capacity;                                                                       \
        hashset->tombstones = 0;                                                                                \
                                                                                                                \
        return 1;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    u8 HASHSET_##K##_quick_add(HASHSET_##K* hashset, const u32 hash, const K key) {                             \
        if (hashset == NULL) return 0;                                                                          \
                                                                                                                \
        if ((hashset->size + 0.0) / hashset->capacity >= HASHSET_MAX_LOAD_FACTOR) {                             \
            const u8 r = HASHSET_##K##_grow(hashset);                                                           \
            if (r == 0) return 0;                                                                               \
        }                                                                                                       \
                                                                                                                \
        u32 i = hash % hashset->capacity;                                                                       \
                                                                                                                \
        HASHSET_ENTRY_##K* found_entry = NULL;                                                                  \
        do {                                                                                                    \
            HASHSET_ENTRY_##K* entry = hashset->entries + i;                                                    \
            const u8 status = entry->status;                                                                    \
                                                                                                                \
            if (status == HASHSET_ENTRY_STATUS_EMPTY || status == HASHSET_ENTRY_STATUS_TOMBSTONE) {             \
                found_entry = entry;                                                                            \
                break;                                                                                          \
            }                                                                                                   \
                                                                                                                \
            if (entry->key == key) break;                                                                       \
            i = (i + 1) % hashset->capacity;                                                                    \
        } while (i != hash % hashset->capacity);                                                                \
                                                                                                                \
        if (found_entry == NULL) return 0;                                                                      \
                                                                                                                \
        found_entry->status = HASHSET_ENTRY_STATUS_FILLED;                                                      \
        found_entry->hash = hash;                                                                               \
        found_entry->key = key;                                                                                 \
                                                                                                                \
        hashset->size++;                                                                                        \
        return 1;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    u8 HASHSET_##K##_add(HASHSET_##K* hashset, const K key) {                                                   \
        if (hashset == NULL) return 0;                                                                          \
                                                                                                                \
        const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));                                                  \
        return HASHSET_##K##_quick_add(hashset, hash, key);                                                     \
    }                                                                                                           \
                                                                                                                \
    void HASHSET_##K##_remove(HASHSET_##K* hashset, const K key) {                                              \
        if (hashset == NULL) return;                                                                            \
                                                                                                                \
        HASHSET_ENTRY_##K* entry = HASHSET_##K##_find(hashset, key);                                            \
        if (entry == NULL) return;                                                                              \
                                                                                                                \
        memset(entry, 0, sizeof(HASHSET_ENTRY_##K));                                                            \
        entry->status = HASHSET_ENTRY_STATUS_TOMBSTONE;                                                         \
        hashset->size--;                                                                                        \
        hashset->tombstones++;                                                                                  \
    }                                                                                                           \
                                                                                                                \
    u32 HASHSET_##K##_hash(const u8* data, const u32 size) {                                                    \
        return HASH_fnv1a(data, size);                                                                          \
    }                                                                                                           \
                                                                                                                \
    u8 HASHSET_##K##_contains(const HASHSET_##K* hashset, const K key) {                                        \
        if (hashset == NULL) return 0;                                                                          \
                                                                                                                \
        const HASHSET_ENTRY_##K* entry = HASHSET_##K##_find(hashset, key);                                      \
        if (entry == NULL) return 0;                                                                            \
        return 1;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    HASHSET_ENTRY_##K* HASHSET_##K##_find(const HASHSET_##K* hashset, const K key) {                            \
        if (hashset == NULL) return NULL;                                                                       \
                                                                                                                \
        const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));                                                  \
        u32 i = hash % hashset->capacity;                                                                       \
                                                                                                                \
        HASHSET_ENTRY_##K* entry;                                                                               \
        u8 status;                                                                                              \
                                                                                                                \
        do {                                                                                                    \
            entry = hashset->entries + i;                                                                       \
            status = entry->status;                                                                             \
                                                                                                                \
            if (status == HASHSET_ENTRY_STATUS_EMPTY) return NULL;                                              \
            if (status == HASHSET_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {                              \
                i = (i + 1) % hashset->capacity;                                                                \
                continue;                                                                                       \
            }                                                                                                   \
                                                                                                                \
            if (entry->key == key) break;                                                                       \
            i = (i + 1) % hashset->capacity;                                                                    \
        } while (i != hash % hashset->capacity);                                                                \
                                                                                                                \
        if (status == HASHSET_ENTRY_STATUS_FILLED) return entry;                                                \
        return NULL;                                                                                            \
    }                                                                                                           \
                                                                                                                \
    HASHSET_##K* HASHSET_##K##_union(const HASHSET_##K* a, const HASHSET_##K* b) {                              \
        if (a == NULL || b == NULL) return NULL;                                                                \
                                                                                                                \
        HASHSET_##K* c = HASHSET_##K##_create(a->capacity + b->capacity);                                       \
        if (c == NULL) return NULL;                                                                             \
                                                                                                                \
        for (u32 ai = 0; ai < a->capacity; ai++) {                                                              \
            const HASHSET_ENTRY_##K entry = a->entries[ai];                                                     \
            if (entry.status == HASHSET_ENTRY_STATUS_FILLED)                                                    \
                HASHSET_##K##_quick_add(c, entry.hash, entry.key);                                              \
        }                                                                                                       \
                                                                                                                \
        for (u32 bi = 0; bi < b->capacity; bi++) {                                                              \
            const HASHSET_ENTRY_##K entry = b->entries[bi];                                                     \
            if (entry.status == HASHSET_ENTRY_STATUS_FILLED)                                                    \
                HASHSET_##K##_quick_add(c, entry.hash, entry.key);                                              \
        }                                                                                                       \
                                                                                                                \
        return c;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    HASHSET_##K* HASHSET_##K##_intersection(const HASHSET_##K* a, const HASHSET_##K* b) {                       \
        if (a == NULL || b == NULL) return NULL;                                                                \
                                                                                                                \
        const HASHSET_##K* smaller;                                                                             \
        const HASHSET_##K* larger;                                                                              \
        if (a->capacity < b->capacity) {                                                                        \
            smaller = a;                                                                                        \
            larger = b;                                                                                         \
        }                                                                                                       \
        else {                                                                                                  \
            smaller = b;                                                                                        \
            larger = a;                                                                                         \
        }                                                                                                       \
                                                                                                                \
        HASHSET_##K* c = HASHSET_##K##_create(smaller->capacity);                                               \
        if (c == NULL) return NULL;                                                                             \
                                                                                                                \
        for (u32 i = 0; i < smaller->capacity; i++) {                                                           \
            const HASHSET_ENTRY_##K entry = smaller->entries[i];                                                \
            if (entry.status == HASHSET_ENTRY_STATUS_FILLED) {                                                  \
                if (HASHSET_##K##_contains(larger, entry.key) == 1) {                                           \
                    HASHSET_##K##_quick_add(c, entry.hash, entry.key);                                          \
                }                                                                                               \
            }                                                                                                   \
        }                                                                                                       \
                                                                                                                \
        return c;                                                                                               \
    }                                                                                                           \
                                                                                                                \
    HASHSET_##K* HASHSET_##K##_difference(const HASHSET_##K* a, const HASHSET_##K* b) {                         \
        if (a == NULL || b == NULL) return NULL;                                                                \
                                                                                                                \
        HASHSET_##K* c = HASHSET_##K##_create(a->capacity);                                                     \
        if (c == NULL) return NULL;                                                                             \
                                                                                                                \
        for (u32 ai = 0; ai < a->capacity; ai++) {                                                              \
            const HASHSET_ENTRY_##K entry = a->entries[ai];                                                     \
            if (entry.status == HASHSET_ENTRY_STATUS_FILLED) {                                                  \
                if (HASHSET_##K##_contains(b, entry.key) == 0) {                                                \
                    HASHSET_##K##_quick_add(c, entry.hash, entry.key);                                          \
                }                                                                                               \
            }                                                                                                   \
        }                                                                                                       \
                                                                                                                \
        return c;                                                                                               \
    }

#endif // NESQUIK_HASHSET_H
