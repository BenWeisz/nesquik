#ifndef NESQUIK_LIST_H
#define NESQUIK_LIST_H

#include "types.h"

#define LIST_MIN_CAPACITY 8

typedef struct LIST {
    u32 size;
    u32 capacity;
    u8* data;
} LIST;

u8 LIST_init(LIST* list, u32 capacity);
LIST* LIST_create(u32 capacity);

void LIST_deinit(LIST* list);
void LIST_destroy(LIST* list);

u8 LIST_grow_capacity(LIST* list);

u8 LIST_push(LIST* list, u8 v);
u8 LIST_pop(LIST* list);
u8 LIST_popv(LIST* list, u8* v);
u8 LIST_get(const LIST* list, u32 i, u8* v);
u8 LIST_set(const LIST* list, u32 i, u8 v);

#endif //NESQUIK_LIST_H