#ifndef ARKIN_TEST_H
#define ARKIN_TEST_H

#include "arkin_core.h"

typedef struct at_case_result_t at_case_result_t;
struct at_case_result_t {
    u32_t line;
    const char *file;
    const char *msg;
    b8_t passed;
};

typedef at_case_result_t (*at_test_func_t)(void);

typedef struct at_case_t at_case_t;
struct at_case_t {
    at_case_t *next;
    at_case_result_t result;
    const char *name;
    at_test_func_t func;
};

typedef struct at_result_t at_result_t;
struct at_result_t {
    at_case_t *cases;
    u32_t passed;
    u32_t failed;
};

typedef struct at_test_t at_test_t;
struct at_test_t {
    at_test_t *next;
    at_test_func_t func;
};

typedef struct at_state_t at_state_t;
struct at_state_t {
    at_case_t *end;
    at_case_t *cases;
};

ARKIN_API at_state_t at_begin(void);
ARKIN_API at_result_t at_end(at_state_t state);
ARKIN_API void at_result_free(at_result_t *result);

#define AT_RUN_TEST(STATE, TEST) _at_run_test(STATE, TEST, #TEST)

#define AT_SUCCESS() return (at_case_result_t) { .passed = true, .msg = NULL, .file = __FILE__, .line = __LINE__ }
#define AT_ASSERT_MSG(VALUE, MSG) if (!VALUE) { return (at_case_result_t) { .passed = VALUE, .msg = MSG, .file = __FILE__, .line = __LINE__ }; }
#define AT_ASSERT(VALUE) AT_ASSERT_MSG(VALUE, NULL)

ARKIN_API void _at_run_test(at_state_t *state, at_test_func_t func, const char *name);

#endif
