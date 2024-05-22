#include "arkin_test.h"

ArTestState ar_test_begin(void) {
    ArTestState state = {0};
    return state;
}

ArTestResult ar_test_end(ArTestState state) {
    ArTestResult res = {
        .cases = state.cases,
    };

    for (ArTestCase *c = state.cases; c != NULL; c = c->next) {
        c->result = c->func();
        if (c->result.passed) {
            res.passed++;
        } else {
            res.failed++;
        }
    }

    return res;
}

void ar_test_result_free(ArTestResult *result) {
    *result = (ArTestResult) {0};
    AR_FREE(result->cases);
}

void _ar_run_test(ArTestState *state, ArTestFunc func, const char *name) {
    ArTestCase *c = AR_MALLOC(sizeof(ArTestCase));
    *c = (ArTestCase) {
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
