#include "test.h"

typedef struct foobar_t foobar_t;
struct foobar_t {
    u32_t foo;
    void *bar;
    f32_t baz;
};

at_case_result_t test_offsetof(void) {
    AT_ASSERT(offsetof(foobar_t, foo) == 0);
    AT_ASSERT(offsetof(foobar_t, bar) == 8);
    AT_ASSERT(offsetof(foobar_t, baz) == 16);

    AT_SUCCESS();
}

at_case_result_t test_arrlen(void) {
    foobar_t arr8[8] = {0};
    AT_ASSERT(arrlen(arr8) == 8);

    foobar_t arr4[4] = {0};
    AT_ASSERT(arrlen(arr4) == 4);

    AT_SUCCESS();
}

at_case_result_t test_constants(void) {
    AT_ASSERT(NULL == (void *) 0);
    AT_ASSERT(true == 1);
    AT_ASSERT(false == 0);

    AT_SUCCESS();
}

at_case_result_t test_sizes(void) {
    AT_ASSERT(sizeof(u8_t)  == 1);
    AT_ASSERT(sizeof(u16_t) == 2);
    AT_ASSERT(sizeof(u32_t) == 4);
    AT_ASSERT(sizeof(u64_t) == 8);

    AT_ASSERT(sizeof(i8_t)  == 1);
    AT_ASSERT(sizeof(i16_t) == 2);
    AT_ASSERT(sizeof(i32_t) == 4);
    AT_ASSERT(sizeof(i64_t) == 8);

    AT_ASSERT(sizeof(f32_t) == 4);
    AT_ASSERT(sizeof(f64_t) == 8);

    AT_ASSERT(sizeof(b8_t)  == 1);
    AT_ASSERT(sizeof(b32_t) == 4);

    AT_SUCCESS();
}

at_result_t test_core(void) {
    at_state_t state = at_begin();

    AT_RUN_TEST(&state, test_offsetof);
    AT_RUN_TEST(&state, test_arrlen);
    AT_RUN_TEST(&state, test_constants);
    AT_RUN_TEST(&state, test_sizes);

    return at_end(state);
}
