#include "arkin_core.h"
#include "arkin_test.h"
#include "test.h"

typedef struct Foobar Foobar;
struct Foobar {
    U32 foo;
    void *bar;
    F32 baz;
};

ArCaseResult test_offsetof(void) {
    AR_ASSERT(ar_offsetof(Foobar, foo) == 0);
    AR_ASSERT(ar_offsetof(Foobar, bar) == 8);
    AR_ASSERT(ar_offsetof(Foobar, baz) == 16);

    AR_SUCCESS();
}

ArCaseResult test_arrlen(void) {
    Foobar arr8[8] = {0};
    AR_ASSERT(ar_arrlen(arr8) == 8);

    Foobar arr4[4] = {0};
    AR_ASSERT(ar_arrlen(arr4) == 4);

    AR_SUCCESS();
}

ArCaseResult test_constants(void) {
    AR_ASSERT(NULL == (void *) 0);
    AR_ASSERT(true == 1);
    AR_ASSERT(false == 0);

    AR_SUCCESS();
}

ArCaseResult test_sizes(void) {
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

ArCaseResult test_page_size(void) {
#ifdef ARKIN_OS_LINUX
    #include <unistd.h>

    AR_ASSERT(ar_os_page_size() == getpagesize());

    AR_SUCCESS();
#endif

    AR_ASSERT_MSG(false, "OS not supported.");
}

ArResult test_core(void) {
    ArState state = ar_begin();

    AR_RUN_TEST(&state, test_offsetof);
    AR_RUN_TEST(&state, test_arrlen);
    AR_RUN_TEST(&state, test_constants);
    AR_RUN_TEST(&state, test_sizes);
    AR_RUN_TEST(&state, test_page_size);

    return ar_end(state);
}
