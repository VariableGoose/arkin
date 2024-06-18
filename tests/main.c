#include "arkin_core.h"
#include "arkin_log.h"

#include "test.h"

static void check(ArTestResult res) {
    for (const ArTestCase *c = res.cases; c != NULL; c = c->next) {
        if (c->result.passed) {
            ar_info("%s: passed", c->name);
        } else {
            if (c->result.msg) {
                ar_error("%s: failed at %s:%u (%s)", c->name, c->result.file, c->result.line, c->result.msg);
            } else {
                ar_error("%s: failed at %s:%u", c->name, c->result.file, c->result.line);
            }
        }
    }
    printf("\n");
}

I32 main(void) {
    arkin_init(&(ArkinCoreDesc) {0});
    ArArena *arena = ar_arena_create_default();

    check(test_core(arena));
    check(test_ll(arena));
    check(test_strings(arena));
    check(test_hash_map(arena));
    check(test_pool(arena));

    ar_arena_destroy(&arena);
    arkin_terminate();
    return 0;
}
