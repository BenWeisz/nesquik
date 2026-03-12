#ifndef NESQUIK_HASHTABLE_H
#define NESQUIK_HASHTABLE_H

#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "hash/hash.h"

#define HASHTABLE_ENTRY_STATUS_EMPTY        0
#define HASHTABLE_ENTRY_STATUS_FILLED       1
#define HASHTABLE_ENTRY_STATUS_TOMBSTONE    2

#define HASHTABLE_MIN_LOAD_FACTOR           0.5
#define HASHTABLE_MAX_LOAD_FACTOR           0.75

#define HASHTABLE_MIN_CAPACITY              8

#pragma pack(push, 1)
#define HASHTABLE_DECLARE(K, V)                                                                                         \
    typedef struct HASHTABLE_ENTRY_##K##_##V {                                                                          \
        u8 status;                                                                                                      \
        u32 hash;                                                                                                       \
        K key;                                                                                                          \
        V value;                                                                                                        \
    } HASHTABLE_ENTRY_##K##_##V;                                                                                        \
                                                                                                                        \
    typedef struct HASHTABLE_##K##_##V {                                                                                \
        HASHTABLE_ENTRY_##K##_##V* entries;                                                                             \
        u32 size;                                                                                                       \
        u32 capacity;                                                                                                   \
        u32 tombstones;                                                                                                 \
    } HASHTABLE_##K##_##V;                                                                                              \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_init(HASHTABLE_##K##_##V* hashtable, u32 capacity);                                        \
    HASHTABLE_##K##_##V* HASHTABLE_##K##_##V##_create(u32 capacity);                                                    \
                                                                                                                        \
    void HASHTABLE_##K##_##V##_deinit(HASHTABLE_##K##_##V* hashtable);                                                  \
    void HASHTABLE_##K##_##V##_destroy(HASHTABLE_##K##_##V* hashtable);                                                 \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_grow(HASHTABLE_##K##_##V* hashtable);                                                      \
    u8 HASHTABLE_##K##_##V##_add(HASHTABLE_##K##_##V* hashtable, K key, V value);                                       \
    u8 HASHTABLE_##K##_##V##_quick_add(HASHTABLE_##K##_##V* hashtable, u32 hash, K key, V value);                       \
    void HASHTABLE_##K##_##V##_remove(HASHTABLE_##K##_##V* hashtable, K key);                                           \
                                                                                                                        \
    u32 HASHTABLE_##K##_##V##_hash(const u8* data, u32 size);                                                           \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_contains(const HASHTABLE_##K##_##V* hashtable, K key);                                     \
    HASHTABLE_ENTRY_##K##_##V* HASHTABLE_##K##_##V##_find(const HASHTABLE_##K##_##V* hashtable, K key);
#pragma pack(pop)

#define HASHTABLE_DEFINE(K, V)                                                                                          \
    u8 HASHTABLE_##K##_##V##_init(HASHTABLE_##K##_##V* hashtable, const u32 capacity) {                                 \
                                                                                                                        \
        if (hashtable == NULL) return 0;                                                                                \
                                                                                                                        \
        hashtable->size = 0;                                                                                            \
        hashtable->capacity = capacity < HASHTABLE_MIN_CAPACITY ? HASHTABLE_MIN_CAPACITY : capacity;                    \
        hashtable->tombstones = 0;                                                                                      \
                                                                                                                        \
        hashtable->entries = (HASHTABLE_ENTRY_##K##_##V*)malloc(sizeof(HASHTABLE_ENTRY_##K##_##V) * capacity);          \
        if (hashtable->entries == NULL) {                                                                               \
            hashtable->capacity = 0;                                                                                    \
            return 0;                                                                                                   \
        }                                                                                                               \
                                                                                                                        \
        memset(hashtable->entries, 0, sizeof(HASHTABLE_ENTRY_##K##_##V) * capacity);                                    \
                                                                                                                        \
        return 1;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    HASHTABLE_##K##_##V* HASHTABLE_##K##_##V##_create(const u32 capacity) {                                             \
                                                                                                                        \
        HASHTABLE_##K##_##V* hashtable = (HASHTABLE_##K##_##V*)malloc(sizeof(HASHTABLE_##K##_##V));                     \
        if (hashtable == NULL) return NULL;                                                                             \
                                                                                                                        \
        const u8 r = HASHTABLE_##K##_##V##_init(hashtable, capacity);                                                   \
        if (r == 0) {                                                                                                   \
            free(hashtable);                                                                                            \
            return NULL;                                                                                                \
        }                                                                                                               \
                                                                                                                        \
        return hashtable;                                                                                               \
    }                                                                                                                   \
                                                                                                                        \
    void HASHTABLE_##K##_##V##_deinit(HASHTABLE_##K##_##V* hashtable) {                                                 \
        if (hashtable == NULL) return;                                                                                  \
        hashtable->size = 0;                                                                                            \
        hashtable->capacity = 0;                                                                                        \
        hashtable->tombstones = 0;                                                                                      \
                                                                                                                        \
        if (hashtable->entries != NULL) {                                                                               \
            free(hashtable->entries);                                                                                   \
            hashtable->entries = NULL;                                                                                  \
        }                                                                                                               \
    }                                                                                                                   \
                                                                                                                        \
    void HASHTABLE_##K##_##V##_destroy(HASHTABLE_##K##_##V* hashtable) {                                                \
        if (hashtable == NULL) return;                                                                                  \
                                                                                                                        \
        HASHTABLE_##K##_##V##_deinit(hashtable);                                                                        \
        free(hashtable);                                                                                                \
    }                                                                                                                   \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_grow(HASHTABLE_##K##_##V* hashtable) {                                                     \
        if (hashtable == NULL) return 0;                                                                                \
                                                                                                                        \
        u32 new_capacity = (u32)(hashtable->capacity * HASHTABLE_MAX_LOAD_FACTOR / HASHTABLE_MIN_LOAD_FACTOR);          \
        new_capacity = (new_capacity > hashtable->capacity) ? new_capacity : hashtable->capacity;                       \
                                                                                                                        \
        HASHTABLE_##K##_##V new_hashtable;                                                                              \
        u8 r = HASHTABLE_##K##_##V##_init(&new_hashtable, new_capacity);                                                \
        if (r == 0) return 0;                                                                                           \
                                                                                                                        \
        for (u32 i = 0; i < hashtable->capacity; i++) {                                                                 \
            const HASHTABLE_ENTRY_##K##_##V* entry = hashtable->entries + i;                                            \
            const u8 status = entry->status;                                                                            \
                                                                                                                        \
            if (status == HASHTABLE_ENTRY_STATUS_EMPTY || status == HASHTABLE_ENTRY_STATUS_TOMBSTONE) continue;         \
            r = HASHTABLE_##K##_##V##_quick_add(&new_hashtable, entry->hash, entry->key, entry->value);                 \
            if (r == 0) {                                                                                               \
                HASHTABLE_##K##_##V##_deinit(&new_hashtable);                                                           \
                return 0;                                                                                               \
            }                                                                                                           \
        }                                                                                                               \
                                                                                                                        \
        free(hashtable->entries);                                                                                       \
        hashtable->entries = new_hashtable.entries;                                                                     \
        hashtable->capacity = new_capacity;                                                                             \
        hashtable->tombstones = 0;                                                                                      \
                                                                                                                        \
        return 1;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_quick_add(HASHTABLE_##K##_##V* hashtable, const u32 hash, const K key, const V value) {    \
        if (hashtable == NULL) return 0;                                                                                \
                                                                                                                        \
        if ((hashtable->size + 0.0) / hashtable->capacity >= HASHTABLE_MAX_LOAD_FACTOR) {                               \
            const u8 r = HASHTABLE_##K##_##V##_grow(hashtable);                                                         \
            if (r == 0) return 0;                                                                                       \
        }                                                                                                               \
                                                                                                                        \
        u32 i = hash % hashtable->capacity;                                                                             \
                                                                                                                        \
        HASHTABLE_ENTRY_##K##_##V* found_entry = NULL;                                                                  \
        do {                                                                                                            \
            HASHTABLE_ENTRY_##K##_##V* entry = hashtable->entries + i;                                                  \
            const u8 status = entry->status;                                                                            \
                                                                                                                        \
            if (status == HASHTABLE_ENTRY_STATUS_EMPTY || status == HASHTABLE_ENTRY_STATUS_TOMBSTONE) {                 \
                found_entry = entry;                                                                                    \
                break;                                                                                                  \
            }                                                                                                           \
                                                                                                                        \
            if (entry->key == key) break;                                                                               \
            i = (i + 1) % hashtable->capacity;                                                                          \
        } while (i != hash % hashtable->capacity);                                                                      \
                                                                                                                        \
        if (found_entry == NULL) return 0;                                                                              \
                                                                                                                        \
        found_entry->status = HASHTABLE_ENTRY_STATUS_FILLED;                                                            \
        found_entry->hash = hash;                                                                                       \
        found_entry->key = key;                                                                                         \
        found_entry->value = value;                                                                                     \
                                                                                                                        \
        hashtable->size++;                                                                                              \
        return 1;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_add(HASHTABLE_##K##_##V* hashtable, const K key, const V value) {                          \
        if (hashtable == NULL) return 0;                                                                                \
                                                                                                                        \
        const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));                                                          \
        return HASHTABLE_##K##_##V##_quick_add(hashtable, hash, key, value);                                            \
    }                                                                                                                   \
                                                                                                                        \
    void HASHTABLE_##K##_##V##_remove(HASHTABLE_##K##_##V* hashtable, const K key) {                                    \
        if (hashtable == NULL) return;                                                                                  \
                                                                                                                        \
        HASHTABLE_ENTRY_##K##_##V* entry = HASHTABLE_##K##_##V##_find(hashtable, key);                                  \
        if (entry == NULL) return;                                                                                      \
                                                                                                                        \
        memset(entry, 0, sizeof(HASHTABLE_ENTRY_##K##_##V));                                                            \
        entry->status = HASHTABLE_ENTRY_STATUS_TOMBSTONE;                                                               \
        hashtable->size--;                                                                                              \
        hashtable->tombstones++;                                                                                        \
    }                                                                                                                   \
                                                                                                                        \
    u32 HASHTABLE_##K##_##V##_hash(const u8* data, const u32 size) {                                                    \
        return HASH_fnv1a(data, size);                                                                                  \
    }                                                                                                                   \
                                                                                                                        \
    u8 HASHTABLE_##K##_##V##_contains(const HASHTABLE_##K##_##V* hashtable, const K key) {                              \
        if (hashtable == NULL) return 0;                                                                                \
                                                                                                                        \
        const HASHTABLE_ENTRY_##K##_##V* entry = HASHTABLE_##K##_##V##_find(hashtable, key);                            \
        if (entry == NULL) return 0;                                                                                    \
        return 1;                                                                                                       \
    }                                                                                                                   \
                                                                                                                        \
    HASHTABLE_ENTRY_##K##_##V* HASHTABLE_##K##_##V##_find(const HASHTABLE_##K##_##V* hashtable, const K key) {          \
        if (hashtable == NULL) return NULL;                                                                             \
                                                                                                                        \
        const u32 hash = HASH_fnv1a((u8*)(&key), sizeof(key));                                                          \
        u32 i = hash % hashtable->capacity;                                                                             \
                                                                                                                        \
        HASHTABLE_ENTRY_##K##_##V* entry;                                                                               \
        u8 status;                                                                                                      \
                                                                                                                        \
        do {                                                                                                            \
            entry = hashtable->entries + i;                                                                             \
            status = entry->status;                                                                                     \
                                                                                                                        \
            if (status == HASHTABLE_ENTRY_STATUS_EMPTY) return NULL;                                                    \
            if (status == HASHTABLE_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {                                    \
                i = (i + 1) % hashtable->capacity;                                                                      \
                continue;                                                                                               \
            }                                                                                                           \
                                                                                                                        \
            if (entry->key == key) break;                                                                               \
            i = (i + 1) % hashtable->capacity;                                                                          \
        } while (i != hash % hashtable->capacity);                                                                      \
                                                                                                                        \
        if (status == HASHTABLE_ENTRY_STATUS_FILLED) return entry;                                                      \
        return NULL;                                                                                                    \
    }

#endif // NESQUIK_HASHTABLE_H
