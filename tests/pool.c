#include "arkin_core.h"
#include "arkin_test.h"
#include "test.h"

ArTestCaseResult test_pool_handle(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    ArPool *pool = ar_pool_init(scratch.arena, 256, sizeof(U32));
    ArPoolHandle handle = ar_pool_handle_create(pool);
    AR_ASSERT(handle.handle == 0);
    AR_ASSERT(ar_pool_handle_valid(pool, handle));

    ar_pool_handle_destroy(pool, handle);
    AR_ASSERT(!ar_pool_handle_valid(pool, handle));

    handle = ar_pool_handle_create(pool);
    AR_ASSERT(handle.handle == (U64) 1 << 32);
    AR_ASSERT(ar_pool_handle_valid(pool, handle));

    handle = ar_pool_handle_create(pool);
    AR_ASSERT(handle.handle == 1);
    AR_ASSERT(ar_pool_handle_valid(pool, handle));

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestCaseResult test_pool_max_capacity(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    ArPool *pool = ar_pool_init(scratch.arena, 1, sizeof(U32));

    ar_pool_handle_create(pool);

    ArPoolHandle handle = ar_pool_handle_create(pool);
    AR_ASSERT(handle.handle == U64_MAX);
    AR_ASSERT(!ar_pool_handle_valid(pool, handle));

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestResult test_pool(ArArena *arena) {
    ArTestState state = ar_test_begin(arena);

    AR_RUN_TEST(&state, test_pool_handle);
    AR_RUN_TEST(&state, test_pool_max_capacity);

    return ar_test_end(state);
}
