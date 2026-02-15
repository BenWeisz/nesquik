#ifndef NESQUIK_STATE_MACHINE_H
#define NESQUIK_STATE_MACHINE_H

#include "types.h"
#include "pointer_hashtable.h"

typedef struct {
    char* name;
    u8 type;
} STATE;

#define STATE_TYPE_START      0
#define STATE_TYPE_INTERNAL   1
#define STATE_TYPE_END        2

#define STATE_CREATE(N, T) \
    STATE N = { .name = #N, .type = T };

typedef STATE* (*TRANSITION_F)(u8* buf, u32 buf_len, void* context);

POINTER_HASHTABLE_DECLARE(STATE, TRANSITION_F)

typedef struct {
    // The buffer to process
    u8* buf;
    u32 buf_len;

    // User context
    void* context;

    // A mapping from states to their transition functions
    POINTER_HASHTABLE_STATE_TRANSITION_F state_transition_table;
} STATE_MACHINE;

u8 STATE_MACHINE_init(STATE_MACHINE* state_machine, u8* buf, const u32 buf_len, void* context);
STATE_MACHINE* STATE_MACHINE_create(u8* buf, const u32 buf_len, void* context);

void STATE_MACHINE_deinit(STATE_MACHINE* state_machine);
void STATE_MACHINE_destroy(STATE_MACHINE* state_machine);

u8 STATE_MACHINE_add_state(STATE_MACHINE* state_machine, STATE* state, TRANSITION_F transition_f);
STATE* STATE_MACHINE_run(STATE_MACHINE* state_machine);

u32 STATE_key_size(const STATE* state);
u8 STATE_key_equal(const STATE* a, const STATE* b);

#endif //NESQUIK_STATE_MACHINE_H