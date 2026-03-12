#include "heap/heap.h"

#include <stdlib.h>

u8 HEAP_init(HEAP* heap, const u32 capacity) {
    if (heap == NULL) return 0;

    heap->size = 0;
    heap->capacity = capacity > HEAP_MIN_CAPACITY ? capacity : HEAP_MIN_CAPACITY;

    heap->data = (HEAP_NODE*) malloc(capacity * sizeof(HEAP_NODE));
    if (heap->data == NULL) {
        heap->capacity = 0;
        return 0;
    }

    return 1;
}

HEAP* HEAP_create(const u32 capacity) {
    HEAP* heap = (HEAP*)malloc(sizeof(HEAP));
    if (heap == NULL) {
        return NULL;
    }

    const u8 r = HEAP_init(heap, capacity);
    if (r == 0) {
        free(heap);
        return NULL;
    }

    return heap;
}

void HEAP_deinit(HEAP* heap) {
    if (heap == NULL) return;

    heap->size = 0;
    heap->capacity = 0;
    if (heap->data != NULL) {
        free(heap->data);
        heap->data = NULL;
    }
}

void HEAP_destroy(HEAP* heap) {
    if (heap == NULL) return;

    HEAP_deinit(heap);
    free(heap);
}

u8 HEAP_grow_capacity(HEAP* heap) {
    if (heap == NULL) return 0;
    if (heap->capacity == 0xFFFFFFFF) return 0;

    u32 capacity = 0;
    if (heap->capacity == 0) {
        capacity = HEAP_MIN_CAPACITY;
    }
    else if (heap->capacity & (1 << 31)) capacity = 0xFFFFFFFF;
    else {
        for (s32 i = 30; i >= 0; i--) {
            if ((1 << i) & capacity) {
                capacity = 1 << (i + 1);
                break;
            }
        }
    }

    HEAP_NODE* data = (HEAP_NODE*)realloc(heap->data, capacity * sizeof(HEAP_NODE));
    if (data == NULL) {
        return 0;
    }

    heap->data = data;
    heap->capacity = capacity;

    return 1;
}

u8 HEAP_swap_nodes(const HEAP* heap, const u32 a, const u32 b) {
    if (heap == NULL) return 0;

    const HEAP_NODE t = heap->data[a];
    heap->data[a] = heap->data[b];
    heap->data[b] = t;

    return 1;
}

u8 HEAP_add(HEAP* heap, const u32 key, const char* value) {
    if (heap == NULL) return 0;
    if (heap->size == 0xFFFFFFFF) return 0;

    if (heap->size + 1 > heap->capacity) {
        const u8 r = HEAP_grow_capacity(heap);
        if (r == 0) return 0;
    }

    HEAP_NODE* node = heap->data + heap->size;
    node->key = key;
    node->value = value;

    u32 ni = heap->size;
    heap->size++;

    while (ni != 0) {
        const u32 pi = ((ni + 1) >> 1) - 1;
        if (heap->data[ni].key > heap->data[pi].key) {
            HEAP_swap_nodes(heap, pi, ni);
            ni = pi;
        }
        else break;
    }

    return 1;
}

u8 HEAP_remove_max(HEAP* heap, HEAP_NODE* result) {
    if (heap == NULL) return 0;

    HEAP_NODE* max_node = HEAP_max(heap);
    result->key = max_node->key;
    result->value = max_node->value;

    max_node->key = heap->data[heap->size - 1].key;
    max_node->value = heap->data[heap->size - 1].value;
    heap->size--;

    u32 ni = 0;
    while (1) {
        const u32 li = 2 * ni + 1;
        const u32 ri = (2 * ni) + 2;

        const u8 li_invalid = li >= heap->size || li < ni;
        const u8 ri_invalid = ri >= heap->size || ri < ni;

        if (li_invalid && ri_invalid) break;

        const u32 nik = heap->data[ni].key;
        if (li_invalid) {
            if (heap->data[ri].key > nik) HEAP_swap_nodes(heap, ni, ri);
        }
        else if (ri_invalid) {
            if (heap->data[li].key > nik) HEAP_swap_nodes(heap, ni, li);
        }
        else {
            const u32 rik = heap->data[ri].key;
            const u32 lik = heap->data[li].key;
            if (nik >= rik && nik >= lik) break;
            if (rik > nik) {
                if (rik >= lik) {
                    HEAP_swap_nodes(heap, ni, ri);
                    ni = ri;
                }
                else {
                    HEAP_swap_nodes(heap, ni, li);
                    ni = li;
                }
            }
            else if (lik > nik) {
                if (lik >= rik) {
                    HEAP_swap_nodes(heap, ni, li);
                    ni = li;
                }
                else {
                    HEAP_swap_nodes(heap, ni, ri);
                    ni = ri;
                }
            }
        }
    }

    return 1;
}

HEAP_NODE* HEAP_max(const HEAP* heap) {
    if (heap == NULL) return NULL;
    return heap->data;
}