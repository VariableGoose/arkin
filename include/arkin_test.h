#ifndef ARKIN_TEST_H
#define ARKIN_TEST_H

#include "arkin_core.h"

typedef struct ArTestCaseResult ArTestCaseResult;
struct ArTestCaseResult {
    U32 line;
    const char *file;
    const char *msg;
    B8 passed;
};

typedef ArTestCaseResult (*ArTestFunc)(void);

typedef struct ArTestCase ArTestCase;
struct ArTestCase {
    ArTestCase *next;
    ArTestCaseResult result;
    const char *name;
    ArTestFunc func;
};

typedef struct ArTestResult ArTestResult;
struct ArTestResult {
    ArTestCase *cases;
    U32 passed;
    U32 failed;
};

typedef struct ArTest ArTest;
struct ArTest {
    ArTest *next;
    ArTestFunc func;
};

typedef struct ArTestState ArTestState;
struct ArTestState {
    ArTestCase *end;
    ArTestCase *cases;
};

ARKIN_API ArTestState ar_test_begin(void);
ARKIN_API ArTestResult ar_test_end(ArTestState state);
ARKIN_API void ar_test_result_free(ArTestResult *result);

#define AR_RUN_TEST(STATE, TEST) _ar_run_test(STATE, TEST, #TEST)

#define AR_SUCCESS() return (ArTestCaseResult) { .passed = true, .msg = NULL, .file = __FILE__, .line = __LINE__ }
#define AR_ASSERT_MSG(VALUE, MSG) if (!(VALUE)) { return (ArTestCaseResult) { .passed = VALUE, .msg = MSG, .file = __FILE__, .line = __LINE__ }; }
#define AR_ASSERT(VALUE) AR_ASSERT_MSG((VALUE), NULL)

ARKIN_API void _ar_run_test(ArTestState *state, ArTestFunc func, const char *name);

#endif
