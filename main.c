#include <stdio.h>

#include "hash/hashtable.h"

HASHTABLE_DECLARE(u64, u64)
HASHTABLE_DEFINE(u64, u64)

int main() {
    HASHTABLE_u64_u64* a = HASHTABLE_u64_u64_create(10);

    HASHTABLE_u64_u64_add(a, 10, 10);
    HASHTABLE_u64_u64_add(a, 12, 10);
    HASHTABLE_u64_u64_add(a, 13, 10);
    HASHTABLE_u64_u64_add(a, 14, 10);

    HASHTABLE_u64_u64_destroy(a);

    return 0;
}

// #include "pointer_hashtable.h"
// #include "state_machine.h"
//
// STATE_CREATE(START, STATE_TYPE_START)
// STATE_CREATE(MID, STATE_TYPE_INTERNAL)
// STATE_CREATE(END, STATE_TYPE_END)
// STATE_CREATE(ERROR, STATE_TYPE_END)
//
// STATE* START_transition(u8* buf, u32 buf_len, void* context) {
//     if (context == NULL) return NULL;
//     u32 i = *(u32*)context;
//
//     if (i == buf_len) return &END;
//     if (i > buf_len) return &ERROR;
//
//     if (buf[i] == 'm') {
//         *(u32*)context = i + 1;
//         return &MID;
//     }
//
//     return &START;
// }
//
// STATE* MID_transition(u8* buf, u32 buf_len, void* context) {
//     if (context == NULL) return NULL;
//     u32 i = *(u32*)context;
//
//     if (i == buf_len) return &END;
//     if (i > buf_len) return &ERROR;
//     if (buf[i] == 'e') {
//         *(u32*)context = i + 1;
//         return &END;
//     }
//
//     return &MID;
// }
//
// int main(void) {
//     const char* test = "me";
//     u32 context = 0;
//
//     STATE_MACHINE* machine = STATE_MACHINE_create((u8*)test, 2, &context);
//     STATE_MACHINE_add_state(machine, &START, START_transition);
//     STATE_MACHINE_add_state(machine, &MID, MID_transition);
//     STATE_MACHINE_add_state(machine, &END, NULL);
//     STATE_MACHINE_add_state(machine, &ERROR, NULL);
//
//     STATE* r = STATE_MACHINE_run(machine);
//
//     STATE_MACHINE_destroy(machine);
//
//     return 0;
// }
