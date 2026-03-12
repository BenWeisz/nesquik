#ifndef NESQUIK_POINTER_HASHSET_H
#define NESQUIK_POINTER_HASHSET_H

#ifndef NESQUIK_TYPES_H
#define NESQUIK_TYPES_H

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;	

typedef float f32;
typedef double f64;

#endif //NESQUIK_TYPES_H

#define POINTER_HASHSET_ENTRY_STATUS_EMPTY      0
#define POINTER_HASHSET_ENTRY_STATUS_FILLED     1
#define POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE  2

#define POINTER_HASHSET_MIN_LOAD_FACTOR         0.5
#define POINTER_HASHSET_MAX_LOAD_FACTOR         0.75

#define POINTER_HASHSET_MIN_CAPACITY            8

#pragma pack(push, 1)
#define POINTER_HASHSET_DECLARE(K)                                                                                          	\
	typedef struct POINTER_HASHSET_##K##_ENTRY {                                                                               	\
	    u8 status;                                                                                                             	\
	    u32 hash;                                                                                                              	\
	    K* key;                                                                                                                	\
	} POINTER_HASHSET_##K##_ENTRY;                                                                                             	\
                                                                                                                            	\
	typedef struct POINTER_HASHSET_##K {                                                                                       	\
	    POINTER_HASHSET_##K##_ENTRY* entries;                                                                                  	\
	    u32 size;                                                                                                              	\
	    u32 capacity;                                                                                                          	\
	    u32 tombstones;                                                                                                        	\
                                                                                                                            	\
	    u32 (*key_size)(const K*);                                                                                             	\
	    u8 (*key_equal)(const K*, const K*);                                                                                   	\
	} POINTER_HASHSET_##K;                                                                                                     	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_init(POINTER_HASHSET_##K* hashset,                                                                	\
	    u32 capacity,                                                                                                          	\
	    u32 (*key_size)(const K*),                                                                                             	\
	    u8 (*key_equal)(const K*, const K*));                                                                                  	\
	POINTER_HASHSET_##K* POINTER_HASHSET_##K##_create(u32 capacity,                                                            	\
	    u32 (*key_size)(const K*),                                                                                             	\
	    u8 (*key_equal)(const K*, const K*));                                                                                  	\
                                                                                                                            	\
	void POINTER_HASHSET_##K##_deinit(POINTER_HASHSET_##K* hashset);                                                           	\
	void POINTER_HASHSET_##K##_destroy(POINTER_HASHSET_##K* hashset);                                                          	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_grow(POINTER_HASHSET_##K* hashset);                                                               	\
	u8 POINTER_HASHSET_##K##_add(POINTER_HASHSET_##K* hashset, K* key);                                                        	\
	u8 POINTER_HASHSET_##K##_quick_add(POINTER_HASHSET_##K* hashset, u32 hash, K* key);                                        	\
	void POINTER_HASHSET_##K##_remove(POINTER_HASHSET_##K* hashset, const K* key);                                             	\
                                                                                                                            	\
	u32 POINTER_HASHSET_##K##_hash(const u8* data, u32 size);                                                                  	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_contains(const POINTER_HASHSET_##K* hashset, const K* key);                                       	\
	POINTER_HASHSET_##K##_ENTRY* POINTER_HASHSET_##K##_find(const POINTER_HASHSET_##K* hashset, const K* key);                 	\
                                                                                                                            	\
	POINTER_HASHSET_##K* POINTER_HASHSET_##K##_union(const POINTER_HASHSET_##K* a, const POINTER_HASHSET_##K* b);              	\
	POINTER_HASHSET_##K* POINTER_HASHSET_##K##_intersection(const POINTER_HASHSET_##K* a, const POINTER_HASHSET_##K* b);       	\
	POINTER_HASHSET_##K* POINTER_HASHSET_##K##_difference(const POINTER_HASHSET_##K* a, const POINTER_HASHSET_##K* b);
#pragma pack(pop)

#include <string.h>
#include <stdlib.h>

#ifndef NESQUIK_HASH_H
#define NESQUIK_HASH_H

#define HASH_FNV32_BASIS 0x811c9dc5
#define HASH_FNV32_PRIME 0x01000193

u32 HASH_fnv1a(const u8* data, u32 size);

u32 HASH_fnv1a(const u8* data, const u32 size) {
    u32 hash = HASH_FNV32_BASIS;
    for (u32 i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= HASH_FNV32_PRIME;
    }

    return hash;
}

#endif //NESQUIK_HASH_H

#define POINTER_HASHSET_DEFINE(K)                                                                                           	\
	u8 POINTER_HASHSET_##K##_init(POINTER_HASHSET_##K* hashset,                                                                	\
	                  const u32 capacity,                                                                                      	\
	                  u32 (*key_size)(const K*),                                                                               	\
	                  u8 (*key_equal)(const K*, const K*)) {                                                                   	\
                                                                                                                            	\
	    if (hashset == NULL) return 0;                                                                                         	\
                                                                                                                            	\
	    hashset->size = 0;                                                                                                     	\
	    hashset->capacity = capacity < POINTER_HASHSET_MIN_CAPACITY ? POINTER_HASHSET_MIN_CAPACITY : capacity;                 	\
	    hashset->tombstones = 0;                                                                                               	\
                                                                                                                            	\
	    hashset->key_size = key_size;                                                                                          	\
	    hashset->key_equal = key_equal;                                                                                        	\
                                                                                                                            	\
	    hashset->entries = (POINTER_HASHSET_##K##_ENTRY*)malloc(sizeof(POINTER_HASHSET_##K##_ENTRY) * capacity);               	\
	    if (hashset->entries == NULL) {                                                                                        	\
	        hashset->capacity = 0;                                                                                             	\
	        return 0;                                                                                                          	\
	    }                                                                                                                      	\
                                                                                                                            	\
	    memset(hashset->entries, 0, sizeof(POINTER_HASHSET_##K##_ENTRY) * capacity);                                           	\
                                                                                                                            	\
	    return 1;                                                                                                              	\
	}                                                                                                                          	\
                                                                                                                            	\
	POINTER_HASHSET_##K* POINTER_HASHSET_##K##_create(const u32 capacity,                                                      	\
	    u32 (*key_size)(const K*),                                                                                             	\
	    u8 (*key_equal)(const K*, const K*)) {                                                                                 	\
                                                                                                                            	\
	    POINTER_HASHSET_##K* hashset = (POINTER_HASHSET_##K*)malloc(sizeof(POINTER_HASHSET_##K));                              	\
	    if (hashset == NULL) return NULL;                                                                                      	\
                                                                                                                            	\
	    const u8 r = POINTER_HASHSET_##K##_init(hashset, capacity, key_size, key_equal);                                       	\
	    if (r == 0) {                                                                                                          	\
	        free(hashset);                                                                                                     	\
	        return NULL;                                                                                                       	\
	    }                                                                                                                      	\
                                                                                                                            	\
	    return hashset;                                                                                                        	\
	}                                                                                                                          	\
                                                                                                                            	\
	void POINTER_HASHSET_##K##_deinit(POINTER_HASHSET_##K* hashset) {                                                          	\
	    if (hashset == NULL) return;                                                                                           	\
	    hashset->size = 0;                                                                                                     	\
	    hashset->capacity = 0;                                                                                                 	\
	    hashset->tombstones = 0;                                                                                               	\
                                                                                                                            	\
	    hashset->key_size = NULL;                                                                                              	\
	    hashset->key_equal = NULL;                                                                                             	\
                                                                                                                            	\
	    if (hashset->entries != NULL) {                                                                                        	\
	        free(hashset->entries);                                                                                            	\
	        hashset->entries = NULL;                                                                                           	\
	    }                                                                                                                      	\
	}                                                                                                                          	\
                                                                                                                            	\
	void POINTER_HASHSET_##K##_destroy(POINTER_HASHSET_##K* hashset) {                                                         	\
	    if (hashset == NULL) return;                                                                                           	\
                                                                                                                            	\
	    POINTER_HASHSET_##K##_deinit(hashset);                                                                                 	\
	    free(hashset);                                                                                                         	\
	}                                                                                                                          	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_grow(POINTER_HASHSET_##K* hashset) {                                                              	\
	    if (hashset == NULL) return 0;                                                                                         	\
                                                                                                                            	\
	    u32 new_capacity = (u32)(hashset->capacity * POINTER_HASHSET_MAX_LOAD_FACTOR / POINTER_HASHSET_MIN_LOAD_FACTOR);       	\
	    new_capacity = (new_capacity > hashset->capacity) ? new_capacity : hashset->capacity;                                  	\
                                                                                                                            	\
	    POINTER_HASHSET_##K new_hashset;                                                                                       	\
	    u8 r = POINTER_HASHSET_##K##_init(&new_hashset, new_capacity, hashset->key_size, hashset->key_equal);                  	\
	    if (r == 0) return 0;                                                                                                  	\
                                                                                                                            	\
	    for (u32 i = 0; i < hashset->capacity; i++) {                                                                          	\
	        const POINTER_HASHSET_##K##_ENTRY* entry = hashset->entries + i;                                                   	\
	        const u8 status = entry->status;                                                                                   	\
                                                                                                                            	\
	        if (status == POINTER_HASHSET_ENTRY_STATUS_EMPTY || status == POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE) continue;    	\
	        r = POINTER_HASHSET_##K##_quick_add(&new_hashset, entry->hash, entry->key);                                        	\
	        if (r == 0) {                                                                                                      	\
	            POINTER_HASHSET_##K##_deinit(&new_hashset);                                                                    	\
	            return 0;                                                                                                      	\
	        }                                                                                                                  	\
	    }                                                                                                                      	\
                                                                                                                            	\
	    free(hashset->entries);                                                                                                	\
	    hashset->entries = new_hashset.entries;                                                                                	\
	    hashset->capacity = new_capacity;                                                                                      	\
	    hashset->tombstones = 0;                                                                                               	\
                                                                                                                            	\
	    return 1;                                                                                                              	\
	}                                                                                                                          	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_quick_add(POINTER_HASHSET_##K* hashset, const u32 hash, K* key) {                                 	\
	    if (hashset == NULL) return 0;                                                                                         	\
                                                                                                                            	\
	    if ((hashset->size + 0.0) / hashset->capacity >= POINTER_HASHSET_MAX_LOAD_FACTOR) {                                    	\
	        const u8 r = POINTER_HASHSET_##K##_grow(hashset);                                                                  	\
	        if (r == 0) return 0;                                                                                              	\
	    }                                                                                                                      	\
                                                                                                                            	\
	    u32 i = hash % hashset->capacity;                                                                                      	\
                                                                                                                            	\
	    POINTER_HASHSET_##K##_ENTRY* found_entry = NULL;                                                                       	\
	    do {                                                                                                                   	\
	        POINTER_HASHSET_##K##_ENTRY* entry = hashset->entries + i;                                                         	\
	        const u8 status = entry->status;                                                                                   	\
                                                                                                                            	\
	        if (status == POINTER_HASHSET_ENTRY_STATUS_EMPTY || status == POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE) {            	\
	            found_entry = entry;                                                                                           	\
	            break;                                                                                                         	\
	        }                                                                                                                  	\
                                                                                                                            	\
	        const u8 equal = hashset->key_equal(entry->key, key);                                                              	\
	        if (equal == 1) break;                                                                                             	\
	        i = (i + 1) % hashset->capacity;                                                                                   	\
	    } while (i != hash % hashset->capacity);                                                                               	\
                                                                                                                            	\
	    if (found_entry == NULL) return 0;                                                                                     	\
                                                                                                                            	\
	    found_entry->status = POINTER_HASHSET_ENTRY_STATUS_FILLED;                                                             	\
	    found_entry->hash = hash;                                                                                              	\
	    found_entry->key = key;                                                                                                	\
                                                                                                                            	\
	    hashset->size++;                                                                                                       	\
	    return 1;                                                                                                              	\
	}                                                                                                                          	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_add(POINTER_HASHSET_##K* hashset, K* key) {                                                       	\
	    if (hashset == NULL) return 0;                                                                                         	\
                                                                                                                            	\
	    const u32 key_size = hashset->key_size(key);                                                                           	\
	    const u32 hash = HASH_fnv1a((u8*)key, key_size);                                                                       	\
                                                                                                                            	\
	    return POINTER_HASHSET_##K##_quick_add(hashset, hash, key);                                                            	\
	}                                                                                                                          	\
                                                                                                                            	\
	void POINTER_HASHSET_##K##_remove(POINTER_HASHSET_##K* hashset, const K* key) {                                            	\
	    if (hashset == NULL) return;                                                                                           	\
                                                                                                                            	\
	    POINTER_HASHSET_##K##_ENTRY* entry = POINTER_HASHSET_##K##_find(hashset, key);                                         	\
	    if (entry == NULL) return;                                                                                             	\
                                                                                                                            	\
	    memset(entry, 0, sizeof(POINTER_HASHSET_##K##_ENTRY));                                                                 	\
	    entry->status = POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE;                                                                	\
	    hashset->size--;                                                                                                       	\
	    hashset->tombstones++;                                                                                                 	\
	}                                                                                                                          	\
                                                                                                                            	\
	u32 POINTER_HASHSET_##K##_hash(const u8* data, const u32 size) {                                                           	\
	    return HASH_fnv1a(data, size);                                                                                         	\
	}                                                                                                                          	\
                                                                                                                            	\
	u8 POINTER_HASHSET_##K##_contains(const POINTER_HASHSET_##K* hashset, const K* key) {                                      	\
	    if (hashset == NULL) return 0;                                                                                         	\
                                                                                                                            	\
	    const POINTER_HASHSET_##K##_ENTRY* entry = POINTER_HASHSET_##K##_find(hashset, key);                                   	\
	    if (entry == NULL) return 0;                                                                                           	\
	    return 1;                                                                                                              	\
	}                                                                                                                          	\
                                                                                                                            	\
	POINTER_HASHSET_##K##_ENTRY* POINTER_HASHSET_##K##_find(const POINTER_HASHSET_##K* hashset, const K* key) {                 \
	    if (hashset == NULL) return NULL;                                                                                      	\
                                                                                                                            	\
	    const u32 key_size = hashset->key_size(key);                                                                           	\
	    const u32 hash = HASH_fnv1a((u8*)key, key_size);                                                                       	\
                                                                                                                            	\
	    u32 i = hash % hashset->capacity;                                                                                      	\
                                                                                                                            	\
	    POINTER_HASHSET_##K##_ENTRY* entry;                                                                                    	\
	    u8 status;                                                                                                             	\
                                                                                                                            	\
	    do {                                                                                                                   	\
	        entry = hashset->entries + i;                                                                                      	\
	        status = entry->status;                                                                                            	\
                                                                                                                            	\
	        if (status == POINTER_HASHSET_ENTRY_STATUS_EMPTY) return NULL;                                                     	\
	        if (status == POINTER_HASHSET_ENTRY_STATUS_TOMBSTONE || entry->hash != hash) {                                     	\
	            i = (i + 1) % hashset->capacity;                                                                               	\
	            continue;                                                                                                      	\
	        }                                                                                                                  	\
                                                                                                                            	\
	        const u8 equal = hashset->key_equal(entry->key, key);                                                              	\
	        if (equal == 1) break;                                                                                             	\
	        i = (i + 1) % hashset->capacity;                                                                                   	\
	    } while (i != hash % hashset->capacity);                                                                               	\
                                                                                                                            	\
	    if (status == POINTER_HASHSET_ENTRY_STATUS_FILLED) return entry;                                                       	\
	    return NULL;                                                                                                           	\
	}

#endif // NESQUIK_POINTER_HASHSET_H
