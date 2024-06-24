#include "arkin_core.h"
#include "arkin_test.h"
#include "test.h"

typedef struct Foobar Foobar;
struct Foobar {
    U32 foo;
    void *bar;
    F32 baz;
};

ArTestCaseResult test_offsetof(void) {
    AR_ASSERT(ar_offsetof(Foobar, foo) == 0);
    AR_ASSERT(ar_offsetof(Foobar, bar) == 8);
    AR_ASSERT(ar_offsetof(Foobar, baz) == 16);

    AR_SUCCESS();
}

ArTestCaseResult test_arrlen(void) {
    Foobar arr8[8] = {0};
    AR_ASSERT(ar_arrlen(arr8) == 8);

    Foobar arr4[4] = {0};
    AR_ASSERT(ar_arrlen(arr4) == 4);

    AR_SUCCESS();
}

ArTestCaseResult test_constants(void) {
    AR_ASSERT(NULL == (void *) 0);
    AR_ASSERT(true == 1);
    AR_ASSERT(false == 0);

    AR_SUCCESS();
}

ArTestCaseResult test_sizes(void) {
    AR_ASSERT(sizeof(U8)  == 1);
    AR_ASSERT(sizeof(U16) == 2);
    AR_ASSERT(sizeof(U32) == 4);
    AR_ASSERT(sizeof(U64) == 8);

    AR_ASSERT(sizeof(I8)  == 1);
    AR_ASSERT(sizeof(I16) == 2);
    AR_ASSERT(sizeof(I32) == 4);
    AR_ASSERT(sizeof(I64) == 8);

    AR_ASSERT(sizeof(F32) == 4);
    AR_ASSERT(sizeof(F64) == 8);

    AR_ASSERT(sizeof(B8)  == 1);
    AR_ASSERT(sizeof(B32) == 4);

    AR_SUCCESS();
}

ArTestCaseResult test_page_size(void) {
#ifdef ARKIN_OS_LINUX
    #include <unistd.h>

    AR_ASSERT(ar_os_page_size() == (U32) getpagesize());

    AR_SUCCESS();
#endif

    AR_ASSERT_MSG(false, "OS not supported.");
}

ArTestCaseResult test_byte_conversion(void) {
    AR_ASSERT(KiB(1) == 1024);
    AR_ASSERT(MiB(1) == 1048576);
    AR_ASSERT(GiB(1) == 1073741824);

    AR_ASSERT(KB(1) == 1000);
    AR_ASSERT(MB(1) == 1000000);
    AR_ASSERT(GB(1) == 1000000000);

    AR_SUCCESS();
}

ArTestResult test_core(ArArena *arena) {
    ArTestState state = ar_test_begin(arena);

    AR_RUN_TEST(&state, test_offsetof);
    AR_RUN_TEST(&state, test_arrlen);
    AR_RUN_TEST(&state, test_constants);
    AR_RUN_TEST(&state, test_sizes);
    AR_RUN_TEST(&state, test_page_size);
    AR_RUN_TEST(&state, test_byte_conversion);

    return ar_test_end(state);
}
