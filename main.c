#include "hash/pointer_hashset.h"
#include <string.h>
#include <stdio.h>

u32 key_size(const char* a) {
    return strlen(a);
}

u8 key_equal(const char* a, const char* b) {
    if (strcmp(a, b) == 0) return 1;
    return 0;
}

int main(void) {
    POINTER_HASHSET* a = POINTER_HASHSET_create(10, key_size, key_equal);

    POINTER_HASHSET_add(a, "a");
    POINTER_HASHSET_add(a, "a");
    POINTER_HASHSET_add(a, "b");
    POINTER_HASHSET_add(a, "c");
    POINTER_HASHSET_add(a, "a");

    printf("%d\n", a->size);

    POINTER_HASHSET_destroy(a);

    return 0;
}