#include "arkin_test.h"

at_state_t at_begin(void) {
    at_state_t state = {0};
    return state;
}

at_result_t at_end(at_state_t state) {
    at_result_t res = {
        .cases = state.cases,
    };

    for (at_case_t *c = state.cases; c != NULL; c = c->next) {
        c->result = c->func();
        if (c->result.passed) {
            res.passed++;
        } else {
            res.failed++;
        }
    }

    return res;
}

void at_result_free(at_result_t *result) {
    *result = (at_result_t) {0};
    AC_FREE(result->cases);
}

void _at_run_test(at_state_t *state, at_test_func_t func, const char *name) {
    at_case_t *c = AC_MALLOC(sizeof(at_case_t));
    *c = (at_case_t) {
        .func = func,
        .name = name,
    };

    if (state->cases == NULL) {
        state->cases = c;
        state->end = state->cases;
        return;
    }

    state->end->next = c;
    state->end = c;
}
