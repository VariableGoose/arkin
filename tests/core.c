#include "arkin_core.h"
#include "arkin_test.h"
#include "test.h"

typedef struct Foobar Foobar;
struct Foobar {
    U32 foo;
    void *bar;
    F32 baz;
};

AtCaseResult test_offsetof(void) {
    AT_ASSERT(offsetof(Foobar, foo) == 0);
    AT_ASSERT(offsetof(Foobar, bar) == 8);
    AT_ASSERT(offsetof(Foobar, baz) == 16);

    AT_SUCCESS();
}

AtCaseResult test_arrlen(void) {
    Foobar arr8[8] = {0};
    AT_ASSERT(arrlen(arr8) == 8);

    Foobar arr4[4] = {0};
    AT_ASSERT(arrlen(arr4) == 4);

    AT_SUCCESS();
}

AtCaseResult test_constants(void) {
    AT_ASSERT(NULL == (void *) 0);
    AT_ASSERT(true == 1);
    AT_ASSERT(false == 0);

    AT_SUCCESS();
}

AtCaseResult test_sizes(void) {
    AT_ASSERT(sizeof(U8)  == 1);
    AT_ASSERT(sizeof(U16) == 2);
    AT_ASSERT(sizeof(U32) == 4);
    AT_ASSERT(sizeof(U64) == 8);

    AT_ASSERT(sizeof(I8)  == 1);
    AT_ASSERT(sizeof(I16) == 2);
    AT_ASSERT(sizeof(I32) == 4);
    AT_ASSERT(sizeof(I64) == 8);

    AT_ASSERT(sizeof(F32) == 4);
    AT_ASSERT(sizeof(F64) == 8);

    AT_ASSERT(sizeof(B8)  == 1);
    AT_ASSERT(sizeof(B32) == 4);

    AT_SUCCESS();
}

AtCaseResult test_page_size(void) {
#ifdef ARKIN_OS_LINUX
    #include <unistd.h>

    AT_ASSERT(ar_os_page_size() == getpagesize());

    AT_SUCCESS();
#endif

    AT_ASSERT_MSG(false, "OS not supported.");
}

AtResult test_core(void) {
    AtState state = at_begin();

    AT_RUN_TEST(&state, test_offsetof);
    AT_RUN_TEST(&state, test_arrlen);
    AT_RUN_TEST(&state, test_constants);
    AT_RUN_TEST(&state, test_sizes);
    AT_RUN_TEST(&state, test_page_size);

    return at_end(state);
}
