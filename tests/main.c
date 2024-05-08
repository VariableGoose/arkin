#include "arkin_log.h"

#include "test.h"

static void check(at_result_t res) {
    for (const at_case_t *c = res.cases; c != NULL; c = c->next) {
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

    at_result_free(&res);
}

i32_t main(void) {
    arkin_init(&(arkin_core_desc_t) {0});

    check(test_core());

    arkin_terminate();
    return 0;
}
