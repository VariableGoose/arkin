#include "arkin_core.h"
#include "arkin_log.h"
#include "arkin_test.h"

at_case_result_t test_func(void) {
    AT_ASSERT_MSG(true, "hehe");
    AT_ASSERT(false);

    AT_SUCCESS();
}

i32_t main(void) {
    arkin_init(&(arkin_core_desc_t) {0});

    at_state_t test_state = at_begin();
    AT_RUN_TEST(&test_state, test_func);

    at_result_t result = at_end(test_state);

    for (const at_case_t *c = result.cases; c != NULL; c = c->next) {
        if (c->result.passed) {
            al_info("%s: passed", c->name);
        } else {
            if (c->result.msg) {
                al_error("%s: failed at %s:%u (%s)", c->name, c->result.file, c->result.line, c->result.msg);
            } else {
                al_error("%s: failed at %s:%u", c->name, c->result.file, c->result.line);
            }
        }
    }

    at_result_free(&result);

    arkin_terminate();
    return 0;
}
