#ifndef NESQUIK_HEAP_H
#define NESQUIK_HEAP_H

#include "types.h"

#define HEAP_MIN_CAPACITY 8

#pragma pack(push, 1)
typedef struct HEAP_NODE {
    u32 key;
    const char* value;
} HEAP_NODE;
#pragma pack(pop)

typedef struct HEAP {
    u32 size;
    u32 capacity;
    HEAP_NODE* data;
} HEAP;

u8 HEAP_init(HEAP* heap, u32 capacity);
HEAP* HEAP_create(u32 capacity);

void HEAP_deinit(HEAP* heap);
void HEAP_destroy(HEAP* heap);

u8 HEAP_grow_capacity(HEAP* heap);
u8 HEAP_swap_nodes(const HEAP* heap, u32 a, u32 b);

u8 HEAP_add(HEAP* heap, u32 key, const char* value);
u8 HEAP_remove_max(HEAP* heap, HEAP_NODE* result);
HEAP_NODE* HEAP_max(const HEAP* heap);

#endif //NESQUIK_HEAP_H