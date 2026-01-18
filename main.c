#include <string.h>
#include "hash/hashset.h"

u32 key_size(const char* k) {
    return strlen(k);
}

u8 key_equal(const char* a, const char* b) {
    if (strcmp(a, b) == 0) return 1;
    return 0;
}

int main(void) {
    HASHSET* a = HASHSET_create(8, key_size,key_equal);
    HASHSET_add(a, "a");
    HASHSET_add(a, "b");

    HASHSET* b = HASHSET_create(8, key_size,key_equal);
    HASHSET_add(b, "c");
    HASHSET_add(b, "c");
    HASHSET_add(b, "b");

    HASHSET* c = HASHSET_difference(a, b);

    HASHSET_destroy(a);
    HASHSET_destroy(b);
    HASHSET_destroy(c);
    return 0;
}