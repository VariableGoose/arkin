#include "arkin_test.h"

ArTestState ar_test_begin(ArArena *arena) {
    ArTestState state = {
        .arena = arena,
    };
    return state;
}

ArTestResult ar_test_end(ArTestState state) {
    ArTestResult res = {
        .cases = state.first,
    };

    for (ArTestCase *c = state.first; c != NULL; c = c->next) {
        c->result = c->func();
        if (c->result.passed) {
            res.passed++;
        } else {
            res.failed++;
        }
    }

    return res;
}

void _ar_run_test(ArTestState *state, ArTestFunc func, const char *name) {
    ArTestCase *test_case = ar_arena_push_type_no_zero(state->arena, ArTestCase);
    *test_case = (ArTestCase) {
        .func = func,
        .name = name,
    };

    ar_sll_queue_push(state->first, state->last, test_case);
}
