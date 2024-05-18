#include "arkin_test.h"

ArState ar_begin(void) {
    ArState state = {0};
    return state;
}

ArResult ar_end(ArState state) {
    ArResult res = {
        .cases = state.cases,
    };

    for (ArCase *c = state.cases; c != NULL; c = c->next) {
        c->result = c->func();
        if (c->result.passed) {
            res.passed++;
        } else {
            res.failed++;
        }
    }

    return res;
}

void ar_result_free(ArResult *result) {
    *result = (ArResult) {0};
    AR_FREE(result->cases);
}

void _ar_run_test(ArState *state, ArTestFunc func, const char *name) {
    ArCase *c = AR_MALLOC(sizeof(ArCase));
    *c = (ArCase) {
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
