#include "arkin_test.h"

AtState at_begin(void) {
    AtState state = {0};
    return state;
}

AtResult at_end(AtState state) {
    AtResult res = {
        .cases = state.cases,
    };

    for (AtCase *c = state.cases; c != NULL; c = c->next) {
        c->result = c->func();
        if (c->result.passed) {
            res.passed++;
        } else {
            res.failed++;
        }
    }

    return res;
}

void at_result_free(AtResult *result) {
    *result = (AtResult) {0};
    AC_FREE(result->cases);
}

void _at_run_test(AtState *state, AtTestFunc func, const char *name) {
    AtCase *c = AC_MALLOC(sizeof(AtCase));
    *c = (AtCase) {
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
