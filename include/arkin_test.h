#ifndef ARKIN_TEST_H
#define ARKIN_TEST_H

#include "arkin_core.h"

typedef struct ArCaseResult ArCaseResult;
struct ArCaseResult {
    U32 line;
    const char *file;
    const char *msg;
    B8 passed;
};

typedef ArCaseResult (*ArTestFunc)(void);

typedef struct ArCase ArCase;
struct ArCase {
    ArCase *next;
    ArCaseResult result;
    const char *name;
    ArTestFunc func;
};

typedef struct ArResult ArResult;
struct ArResult {
    ArCase *cases;
    U32 passed;
    U32 failed;
};

typedef struct ArTest ArTest;
struct ArTest {
    ArTest *next;
    ArTestFunc func;
};

typedef struct ArState ArState;
struct ArState {
    ArCase *end;
    ArCase *cases;
};

ARKIN_API ArState ar_begin(void);
ARKIN_API ArResult ar_end(ArState state);
ARKIN_API void ar_result_free(ArResult *result);

#define AR_RUN_TEST(STATE, TEST) _ar_run_test(STATE, TEST, #TEST)

#define AR_SUCCESS() return (ArCaseResult) { .passed = true, .msg = NULL, .file = __FILE__, .line = __LINE__ }
#define AR_ASSERT_MSG(VALUE, MSG) if (!(VALUE)) { return (ArCaseResult) { .passed = VALUE, .msg = MSG, .file = __FILE__, .line = __LINE__ }; }
#define AR_ASSERT(VALUE) AR_ASSERT_MSG((VALUE), NULL)

ARKIN_API void _ar_run_test(ArState *state, ArTestFunc func, const char *name);

#endif
