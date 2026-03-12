#include "list/list.h"

#include <stdlib.h>

LIST_DECLARE(u8)

u8 LIST_u8_init(LIST_u8* list, u32 capacity) {
    if (list == NULL) return 0;

    list->size = 0;
    list->capacity = capacity < LIST_MIN_CAPACITY ? LIST_MIN_CAPACITY : capacity;

    list->data = (u8*)malloc(sizeof(u8) * list->capacity);
    if (list->data == NULL) {
        list->capacity = 0;
        return 0;
    }

    return 1;
}

LIST_u8* LIST_u8_create(const u32 capacity) {
    LIST_u8* list = (LIST_u8*)malloc(sizeof(LIST_u8));
    if (list == NULL) {
        return NULL;
    }

    const u8 r = LIST_u8_init(list, capacity);
    if (r == 0) {
        free(list);
        return NULL;
    }

    return list;
}

void LIST_u8_deinit(LIST_u8* list) {
    if (list == NULL) return;

    list->size = 0;
    list->capacity = 0;
    if (list->data != NULL) {
        free(list->data);
        list->data = NULL;
    }
}

void LIST_u8_destroy(LIST_u8* list) {
    if (list == NULL) return;

    LIST_u8_deinit(list);
    free(list);
}

u8 LIST_u8_grow_capacity(LIST_u8* list) {
    if (list == NULL) return 0;

    if (list->capacity == 0xFFFFFFFF) return 0;

    u32 capacity = 0;
    if (list->capacity == 0) {
        capacity = LIST_MIN_CAPACITY;
    }
    else if (list->capacity & (1 << 31)) capacity = 0xFFFFFFFF;
    else {
        for (s32 i = 30; i >= 0; i--) {
            if ((1 << i) & list->capacity) {
                capacity = 1 << (i + 1);
                break;
            }
        }
    }

    u8* data = realloc(list->data, sizeof(u8) * capacity);
    if (data == NULL) {
        return 0;
    }
    list->data = data;
    list->capacity = capacity;

    return 1;
}

u8 LIST_u8_push(LIST_u8* list, u8 v) {
    if (list == NULL) return 0;
    if (list->size == 0xFFFFFFFF) return 0;

    if (list->size + 1 > list->capacity) {
        const u8 r = LIST_u8_grow_capacity(list);
        if (r == 0) return 0;
    }

    list->data[list->size++] = v;
    return 1;
}

u8 LIST_u8_pop(LIST_u8* list) {
    if (list == NULL) return 0;
    if (list->size == 0) return 0;

    list->size--;
    return 1;
}

u8 LIST_u8_popv(LIST_u8* list, u8* v) {
    if (list == NULL) return 0;
    if (list->size == 0) return 0;

    *v = list->data[--list->size];
    return 1;
}

u8 LIST_u8_get(const LIST_u8* list, u32 i, u8* v) {
    if (list == NULL) return 0;
    if (i >= list->size) return 0;

    *v = list->data[i];
    return 1;
}

u8 LIST_u8_set(const LIST_u8* list, u32 i, u8 v) {
    if (list == NULL) return 0;
    if (i >= list->size) return 0;

    list->data[i] = v;
    return 1;
}