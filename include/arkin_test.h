#ifndef ARKIN_TEST_H
#define ARKIN_TEST_H

#include "arkin_core.h"

typedef struct AtCaseResult AtCaseResult;
struct AtCaseResult {
    U32 line;
    const char *file;
    const char *msg;
    B8 passed;
};

typedef AtCaseResult (*AtTestFunc)(void);

typedef struct AtCase AtCase;
struct AtCase {
    AtCase *next;
    AtCaseResult result;
    const char *name;
    AtTestFunc func;
};

typedef struct AtResult AtResult;
struct AtResult {
    AtCase *cases;
    U32 passed;
    U32 failed;
};

typedef struct AtTest AtTest;
struct AtTest {
    AtTest *next;
    AtTestFunc func;
};

typedef struct AtState AtState;
struct AtState {
    AtCase *end;
    AtCase *cases;
};

ARKIN_API AtState at_begin(void);
ARKIN_API AtResult at_end(AtState state);
ARKIN_API void at_result_free(AtResult *result);

#define AT_RUN_TEST(STATE, TEST) _at_run_test(STATE, TEST, #TEST)

#define AT_SUCCESS() return (AtCaseResult) { .passed = true, .msg = NULL, .file = __FILE__, .line = __LINE__ }
#define AT_ASSERT_MSG(VALUE, MSG) if (!VALUE) { return (AtCaseResult) { .passed = VALUE, .msg = MSG, .file = __FILE__, .line = __LINE__ }; }
#define AT_ASSERT(VALUE) AT_ASSERT_MSG(VALUE, NULL)

ARKIN_API void _at_run_test(AtState *state, AtTestFunc func, const char *name);

#endif
