#ifndef NESQUIK_POINTER_HASHTABLE_H
#define NESQUIK_POINTER_HASHTABLE_H

#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "hash/hash.h"

#define POINTER_HASHTABLE_ENTRY_STATUS_EMPTY        0
#define POINTER_HASHTABLE_ENTRY_STATUS_FILLED       1
#define POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE    2

#define POINTER_HASHTABLE_MIN_LOAD_FACTOR           0.5
#define POINTER_HASHTABLE_MAX_LOAD_FACTOR           0.75

#define POINTER_HASHTABLE_MIN_CAPACITY              8

#pragma pack(push, 1)
#define POINTER_HASHTABLE_DECLARE(K, V)                                                                                                     \
    typedef struct POINTER_HASHTABLE_ENTRY_##K##_##V {                                                                                      \
        u8 status;                                                                                                                          \
        u32 hash;                                                                                                                           \
        K* key;                                                                                                                             \
        V value;                                                                                                                            \
    } POINTER_HASHTABLE_ENTRY_##K##_##V;                                                                                                    \
                                                                                                                                            \
    typedef struct POINTER_HASHTABLE_##K##_##V {                                                                                            \
        POINTER_HASHTABLE_ENTRY_##K##_##V* entries;                                                                                         \
        u32 size;                                                                                                                           \
        u32 capacity;                                                                                                                       \
        u32 tombstones;                                                                                                                     \
                                                                                                                                            \
        u32 (*key_size)(const K*);                                                                                                          \
        u8 (*key_equal)(const K*, const K*);                                                                                                \
    } POINTER_HASHTABLE_##K##_##V;                                                                                                          \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_init(POINTER_HASHTABLE_##K##_##V* hashtable,                                                           \
        u32 capacity,                                                                                                                       \
        u32 (*key_size)(const K*),                                                                                                          \
        u8 (*key_equal)(const K*, const K*));                                                                                               \
    POINTER_HASHTABLE_##K##_##V* POINTER_HASHTABLE_##K##_##V##_create(u32 capacity,                                                         \
        u32 (*key_size)(const K*),                                                                                                          \
        u8 (*key_equal)(const K*, const K*));                                                                                               \
                                                                                                                                            \
    void POINTER_HASHTABLE_##K##_##V##_deinit(POINTER_HASHTABLE_##K##_##V* hashtable);                                                      \
    void POINTER_HASHTABLE_##K##_##V##_destroy(POINTER_HASHTABLE_##K##_##V* hashtable);                                                     \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_grow(POINTER_HASHTABLE_##K##_##V* hashtable);                                                          \
    u8 POINTER_HASHTABLE_##K##_##V##_add(POINTER_HASHTABLE_##K##_##V* hashtable, K* key, V value);                                          \
    u8 POINTER_HASHTABLE_##K##_##V##_quick_add(POINTER_HASHTABLE_##K##_##V* hashtable, u32 hash, K* key, V value);                          \
    void POINTER_HASHTABLE_##K##_##V##_remove(POINTER_HASHTABLE_##K##_##V* hashtable, const K* key);                                        \
                                                                                                                                            \
    u32 POINTER_HASHTABLE_##K##_##V##_hash(const u8* data, u32 size);                                                                       \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_contains(const POINTER_HASHTABLE_##K##_##V* hashtable, const K* key);                                  \
    POINTER_HASHTABLE_ENTRY_##K##_##V* POINTER_HASHTABLE_##K##_##V##_find(const POINTER_HASHTABLE_##K##_##V* hashtable, const K* key);
#pragma pack(pop)

#define POINTER_HASHTABLE_DEFINE(K, V)                                                                                                      \
    u8 POINTER_HASHTABLE_##K##_##V##_init(POINTER_HASHTABLE_##K##_##V* hashtable,                                                           \
                      const u32 capacity,                                                                                                   \
                      u32 (*key_size)(const K*),                                                                                            \
                      u8 (*key_equal)(const K*, const K*)) {                                                                                \
                                                                                                                                            \
        if (hashtable == NULL) return 0;                                                                                                    \
                                                                                                                                            \
        hashtable->size = 0;                                                                                                                \
        hashtable->capacity = capacity < POINTER_HASHTABLE_MIN_CAPACITY ? POINTER_HASHTABLE_MIN_CAPACITY : capacity;                        \
        hashtable->tombstones = 0;                                                                                                          \
                                                                                                                                            \
        hashtable->key_size = key_size;                                                                                                     \
        hashtable->key_equal = key_equal;                                                                                                   \
                                                                                                                                            \
        hashtable->entries = (POINTER_HASHTABLE_ENTRY_##K##_##V*)malloc(sizeof(POINTER_HASHTABLE_ENTRY_##K##_##V) * capacity);              \
        if (hashtable->entries == NULL) {                                                                                                   \
            hashtable->capacity = 0;                                                                                                        \
            return 0;                                                                                                                       \
        }                                                                                                                                   \
                                                                                                                                            \
        memset(hashtable->entries, 0, sizeof(POINTER_HASHTABLE_ENTRY_##K##_##V) * capacity);                                                \
                                                                                                                                            \
        return 1;                                                                                                                           \
    }                                                                                                                                       \
                                                                                                                                            \
    POINTER_HASHTABLE_##K##_##V* POINTER_HASHTABLE_##K##_##V##_create(const u32 capacity,                                                   \
        u32 (*key_size)(const K*),                                                                                                          \
        u8 (*key_equal)(const K*, const K*)) {                                                                                              \
                                                                                                                                            \
        POINTER_HASHTABLE_##K##_##V* hashtable = (POINTER_HASHTABLE_##K##_##V*)malloc(sizeof(POINTER_HASHTABLE_##K##_##V));                 \
        if (hashtable == NULL) return NULL;                                                                                                 \
                                                                                                                                            \
        const u8 r = POINTER_HASHTABLE_##K##_##V##_init(hashtable, capacity, key_size, key_equal);                                          \
        if (r == 0) {                                                                                                                       \
            free(hashtable);                                                                                                                \
            return NULL;                                                                                                                    \
        }                                                                                                                                   \
                                                                                                                                            \
        return hashtable;                                                                                                                   \
    }                                                                                                                                       \
                                                                                                                                            \
    void POINTER_HASHTABLE_##K##_##V##_deinit(POINTER_HASHTABLE_##K##_##V* hashtable) {                                                     \
        if (hashtable == NULL) return;                                                                                                      \
        hashtable->size = 0;                                                                                                                \
        hashtable->capacity = 0;                                                                                                            \
        hashtable->tombstones = 0;                                                                                                          \
                                                                                                                                            \
        hashtable->key_size = NULL;                                                                                                         \
        hashtable->key_equal = NULL;                                                                                                        \
                                                                                                                                            \
        if (hashtable->entries != NULL) {                                                                                                   \
            free(hashtable->entries);                                                                                                       \
            hashtable->entries = NULL;                                                                                                      \
        }                                                                                                                                   \
    }                                                                                                                                       \
                                                                                                                                            \
    void POINTER_HASHTABLE_##K##_##V##_destroy(POINTER_HASHTABLE_##K##_##V* hashtable) {                                                    \
        if (hashtable == NULL) return;                                                                                                      \
                                                                                                                                            \
        POINTER_HASHTABLE_##K##_##V##_deinit(hashtable);                                                                                    \
        free(hashtable);                                                                                                                    \
    }                                                                                                                                       \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_grow(POINTER_HASHTABLE_##K##_##V* hashtable) {                                                         \
        if (hashtable == NULL) return 0;                                                                                                    \
                                                                                                                                            \
        u32 new_capacity = (u32)(hashtable->capacity * POINTER_HASHTABLE_MAX_LOAD_FACTOR / POINTER_HASHTABLE_MIN_LOAD_FACTOR);              \
        new_capacity = (new_capacity > hashtable->capacity) ? new_capacity : hashtable->capacity;                                           \
                                                                                                                                            \
        POINTER_HASHTABLE_##K##_##V new_hashtable;                                                                                          \
        u8 r = POINTER_HASHTABLE_##K##_##V##_init(&new_hashtable, new_capacity, hashtable->key_size, hashtable->key_equal);                 \
        if (r == 0) return 0;                                                                                                               \
                                                                                                                                            \
        for (u32 i = 0; i < hashtable->capacity; i++) {                                                                                     \
            const POINTER_HASHTABLE_ENTRY_##K##_##V* entry = hashtable->entries + i;                                                        \
            const u8 status = entry->status;                                                                                                \
                                                                                                                                            \
            if (status == POINTER_HASHTABLE_ENTRY_STATUS_EMPTY || status == POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE) continue;             \
            r = POINTER_HASHTABLE_##K##_##V##_quick_add(&new_hashtable, entry->hash, entry->key, entry->value);                             \
            if (r == 0) {                                                                                                                   \
                POINTER_HASHTABLE_##K##_##V##_deinit(&new_hashtable);                                                                       \
                return 0;                                                                                                                   \
            }                                                                                                                               \
        }                                                                                                                                   \
                                                                                                                                            \
        free(hashtable->entries);                                                                                                           \
        hashtable->entries = new_hashtable.entries;                                                                                         \
        hashtable->capacity = new_capacity;                                                                                                 \
        hashtable->tombstones = 0;                                                                                                          \
                                                                                                                                            \
        return 1;                                                                                                                           \
    }                                                                                                                                       \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_quick_add(POINTER_HASHTABLE_##K##_##V* hashtable, const u32 hash, K* key, u32 value) {                 \
        if (hashtable == NULL) return 0;                                                                                                    \
                                                                                                                                            \
        if ((hashtable->size + 0.0) / hashtable->capacity >= POINTER_HASHTABLE_MAX_LOAD_FACTOR) {                                           \
            const u8 r = POINTER_HASHTABLE_##K##_##V##_grow(hashtable);                                                                     \
            if (r == 0) return 0;                                                                                                           \
        }                                                                                                                                   \
                                                                                                                                            \
        u32 i = hash % hashtable->capacity;                                                                                                 \
                                                                                                                                            \
        POINTER_HASHTABLE_ENTRY_##K##_##V* found_entry = NULL;                                                                              \
        do {                                                                                                                                \
            POINTER_HASHTABLE_ENTRY_##K##_##V* entry = hashtable->entries + i;                                                              \
            const u8 status = entry->status;                                                                                                \
                                                                                                                                            \
            if (status == POINTER_HASHTABLE_ENTRY_STATUS_EMPTY || status == POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE) {                     \
                found_entry = entry;                                                                                                        \
                break;                                                                                                                      \
            }                                                                                                                               \
                                                                                                                                            \
            const u8 equal = hashtable->key_equal(entry->key, key);                                                                         \
            if (equal == 1) break;                                                                                                          \
            i = (i + 1) % hashtable->capacity;                                                                                              \
        } while (i != hash % hashtable->capacity);                                                                                          \
                                                                                                                                            \
        if (found_entry == NULL) return 0;                                                                                                  \
                                                                                                                                            \
        found_entry->status = POINTER_HASHTABLE_ENTRY_STATUS_FILLED;                                                                        \
        found_entry->hash = hash;                                                                                                           \
        found_entry->key = key;                                                                                                             \
        found_entry->value = value;                                                                                                         \
                                                                                                                                            \
        hashtable->size++;                                                                                                                  \
        return 1;                                                                                                                           \
    }                                                                                                                                       \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_add(POINTER_HASHTABLE_##K##_##V* hashtable, K* key, u32 value) {                                       \
        if (hashtable == NULL) return 0;                                                                                                    \
                                                                                                                                            \
        const u32 key_size = hashtable->key_size(key);                                                                                      \
        const u32 hash = HASH_fnv1a((u8*)key, key_size);                                                                                    \
                                                                                                                                            \
        return POINTER_HASHTABLE_##K##_##V##_quick_add(hashtable, hash, key, value);                                                        \
    }                                                                                                                                       \
                                                                                                                                            \
    void POINTER_HASHTABLE_##K##_##V##_remove(POINTER_HASHTABLE_##K##_##V* hashtable, const K* key) {                                       \
        if (hashtable == NULL) return;                                                                                                      \
                                                                                                                                            \
        POINTER_HASHTABLE_ENTRY_##K##_##V* entry = POINTER_HASHTABLE_##K##_##V##_find(hashtable, key);                                      \
        if (entry == NULL) return;                                                                                                          \
                                                                                                                                            \
        memset(entry, 0, sizeof(POINTER_HASHTABLE_ENTRY_##K##_##V));                                                                        \
        entry->status = POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE;                                                                           \
        hashtable->size--;                                                                                                                  \
        hashtable->tombstones++;                                                                                                            \
    }                                                                                                                                       \
                                                                                                                                            \
    u32 POINTER_HASHTABLE_##K##_##V##_hash(const u8* data, const u32 size) {                                                                \
        return HASH_fnv1a(data, size);                                                                                                      \
    }                                                                                                                                       \
                                                                                                                                            \
    u8 POINTER_HASHTABLE_##K##_##V##_contains(const POINTER_HASHTABLE_##K##_##V* hashtable, const K* key) {                                 \
        if (hashtable == NULL) return 0;                                                                                                    \
                                                                                                                                            \
        const POINTER_HASHTABLE_ENTRY_##K##_##V* entry = POINTER_HASHTABLE_##K##_##V##_find(hashtable, key);                                \
        if (entry == NULL) return 0;                                                                                                        \
        return 1;                                                                                                                           \
    }                                                                                                                                       \
                                                                                                                                            \
    POINTER_HASHTABLE_ENTRY_##K##_##V* POINTER_HASHTABLE_##K##_##V##_find(const POINTER_HASHTABLE_##K##_##V* hashtable, const K* key) {     \
        if (hashtable == NULL) return NULL;                                                                                                 \
                                                                                                                                            \
        const u32 key_size = hashtable->key_size(key);                                                                                      \
        const u32 hash = HASH_fnv1a((u8*)key, key_size);                                                                                    \
                                                                                                                                            \
        u32 i = hash % hashtable->capacity;                                                                                                 \
                                                                                                                                            \
        POINTER_HASHTABLE_ENTRY_##K##_##V* entry;                                                                                           \
        u8 status;                                                                                                                          \
                                                                                                                                            \
        do {                                                                                                                                \
            entry = hashtable->entries + i;                                                                                                 \
            status = entry->status;                                                                                                         \
                                                                                                                                            \
            if (status == POINTER_HASHTABLE_ENTRY_STATUS_EMPTY) return NULL;                                                                \
            if (status == POINTER_HASHTABLE_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {                                                \
                i = (i + 1) % hashtable->capacity;                                                                                          \
                continue;                                                                                                                   \
            }                                                                                                                               \
                                                                                                                                            \
            const u8 equal = hashtable->key_equal(entry->key, key);                                                                         \
            if (equal == 1) break;                                                                                                          \
            i = (i + 1) % hashtable->capacity;                                                                                              \
        } while (i != hash % hashtable->capacity);                                                                                          \
                                                                                                                                            \
        if (status == POINTER_HASHTABLE_ENTRY_STATUS_FILLED) return entry;                                                                  \
        return NULL;                                                                                                                        \
    }

#endif // NESQUIK_POINTER_HASHTABLE_H
