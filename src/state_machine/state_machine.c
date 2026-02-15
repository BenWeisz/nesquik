#include "state_machine/state_machine.h"

#include <stdlib.h>

POINTER_HASHTABLE_DEFINE(STATE, TRANSITION_F)

u8 STATE_MACHINE_init(STATE_MACHINE* state_machine, u8* buf, const u32 buf_len, void* context) {
    if (state_machine == NULL) return 0;

    state_machine->buf = buf;
    state_machine->buf_len = buf_len;
    state_machine->context = context;

    const u8 r = POINTER_HASHTABLE_STATE_TRANSITION_F_init(
        &(state_machine->state_transition_table), 8,
        STATE_key_size, STATE_key_equal);

    if (r == 0) return 0;

    return 1;
}

STATE_MACHINE* STATE_MACHINE_create(u8* buf, const u32 buf_len, void* context) {
    STATE_MACHINE* state_machine = (STATE_MACHINE*)malloc(sizeof(STATE_MACHINE));
    if (state_machine == NULL) return NULL;

    const u8 r = STATE_MACHINE_init(state_machine, buf, buf_len, context);
    if (r == 0) {
        free(state_machine);
        return NULL;
    }

    return state_machine;
}

void STATE_MACHINE_deinit(STATE_MACHINE* state_machine) {
    if (state_machine == NULL) return;

    state_machine->buf = NULL;
    state_machine->buf_len = 0;
    state_machine->context = NULL;
    POINTER_HASHTABLE_STATE_TRANSITION_F_deinit(&(state_machine->state_transition_table));
}

void STATE_MACHINE_destroy(STATE_MACHINE* state_machine) {
    if (state_machine == NULL) return;
    STATE_MACHINE_deinit(state_machine);
}

u8 STATE_MACHINE_add_state(STATE_MACHINE* state_machine, STATE* state, TRANSITION_F transition_f) {
    if (state_machine == NULL) return 0;

    const u8 r = POINTER_HASHTABLE_STATE_TRANSITION_F_add(&(state_machine->state_transition_table), state, transition_f);
    if (r == 0) return 0;
    return 1;
}

STATE* STATE_MACHINE_run(STATE_MACHINE* state_machine) {
    if (state_machine == NULL) return NULL;

    STATE* curr_state = NULL;

    // Find the start state
    for (u32 i = 0; i < state_machine->state_transition_table.capacity; i++) {
        const POINTER_HASHTABLE_ENTRY_STATE_TRANSITION_F entry = state_machine->state_transition_table.entries[i];
        if (entry.status == POINTER_HASHTABLE_ENTRY_STATUS_FILLED) {
            if (entry.key->type == STATE_TYPE_START)
                curr_state = entry.key;
        }
    }

    // We couldn't find a start state, PANICK!!!
    if (curr_state == NULL) return NULL;

    // Consume the buffer
    while (curr_state->type != STATE_TYPE_END) {
        const POINTER_HASHTABLE_ENTRY_STATE_TRANSITION_F* entry = POINTER_HASHTABLE_STATE_TRANSITION_F_find(
            &(state_machine->state_transition_table), curr_state);

        // What kind of state is this???
        if (entry == NULL) return NULL;

        const TRANSITION_F transition_f = entry->value;
        // The function pointer is corrupted, not set, or this we are trying to transition from an end state?
        if (transition_f == NULL) return NULL;

        curr_state = transition_f(state_machine->buf, state_machine->buf_len, state_machine->context);

        // Something went wrong, we didn't transition correctly
        if (curr_state == NULL) return NULL;
    }

    return curr_state;
}

u32 STATE_key_size(const STATE* state) {
    return sizeof(STATE);
}

u8 STATE_key_equal(const STATE* a, const STATE* b) {
    return a->name == b->name && a->type == b->type;
}