#include "arkin_core.h"
#include "arkin_test.h"
#include "test.h"

static U64 str_hash(const void *data, U64 len) {
    (void) len;
    const ArStr *str = data;
    return ar_fvn1a_hash(str->data, str->len);
}

static B8 str_cmp(const void *a, const void *b, U64 len) {
    const ArStr *_a = a;
    const ArStr *_b = b;
    return ar_str_match(*_a, *_b, AR_STR_MATCH_FLAG_EXACT);
}

ArTestCaseResult test_hash_map_insert(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    ar_hash_map_insert(map, ar_str_lit("foo"), 42);
    ar_hash_map_insert(map, ar_str_lit("bar"), 24);

    U32 value = ar_hash_map_get(map, ar_str_lit("foo"), U32);
    AR_ASSERT(value == 42)
    value = ar_hash_map_get(map, ar_str_lit("bar"), U32);
    AR_ASSERT(value == 24)

    B8 success = ar_hash_map_remove(map, ar_str_lit("foo"));
    AR_ASSERT(success);

    value = ar_hash_map_get(map, ar_str_lit("foo"), U32);
    AR_ASSERT(value == null_value)
    value = ar_hash_map_get(map, ar_str_lit("bar"), U32);
    AR_ASSERT(value == 24)

    success = ar_hash_map_remove(map, ar_str_lit("foo"));
    AR_ASSERT(!success);

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestCaseResult test_hash_map_set(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    U8 unique = ar_hash_map_set(map, ar_str_lit("foobar"), 42);
    AR_ASSERT_MSG(unique, "Unique key set but wasn't returned as unique.");

    U32 value = ar_hash_map_get(map, ar_str_lit("foobar"), U32);
    AR_ASSERT(value == 42);

    unique = ar_hash_map_set(map, ar_str_lit("foobar"), 24);
    AR_ASSERT_MSG(!unique, "Non-unique key set but was returned as unique.");

    value = ar_hash_map_get(map, ar_str_lit("foobar"), U32);
    AR_ASSERT(value == 24);

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestCaseResult test_hash_map_remove(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestCaseResult test_hash_map_has(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    ar_hash_map_insert(map, ar_str_lit("foobar"), 42);

    U8 has = ar_hash_map_has(map, ar_str_lit("foobar"));
    AR_ASSERT_MSG(has, "Key should be present in hash map.");

    has = ar_hash_map_has(map, ar_str_lit("something else"));
    AR_ASSERT_MSG(!has, "Key should not be present in hash map.");

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestCaseResult test_hash_map_get(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    ar_hash_map_insert(map, ar_str_lit("foobar"), 42);

    U32 value = ar_hash_map_get(map, ar_str_lit("foobar"), U32);
    AR_ASSERT(value == 42);

    value = ar_hash_map_get(map, ar_str_lit("qux"), U32);
    AR_ASSERT(value == null_value);

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestCaseResult test_hash_map_get_ptr(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    ar_hash_map_insert(map, ar_str_lit("foobar"), 42);

    U32 *value = ar_hash_map_get_ptr(map, ar_str_lit("foobar"));
    AR_ASSERT(*value == 42);

    value = ar_hash_map_get_ptr(map, ar_str_lit("qux"));
    AR_ASSERT(value == NULL);

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

#include "arkin_log.h"
ArTestCaseResult test_hash_map_iter(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    // Key: ArStr
    // Value: U32
    U32 null_value = ~0;
    ArHashMap *map = ar_hash_map_init((ArHashMapDesc) {
            .arena = scratch.arena,
            .capacity = 16,

            .eq_func = str_cmp,
            .hash_func = str_hash,

            .key_size = sizeof(ArStr),
            .value_size = sizeof(U32),
            .null_value = &null_value,
        });

    // These have to be in this order because that's the way the keys get hashed and
    // places in memory.
    ArStr keys[] = {
        ar_str_lit("foo"),
        ar_str_lit("qux"),
        ar_str_lit("bar"),
    };
    U32 values[] = {
        22,
        32,
        42,
    };

    ar_hash_map_insert(map, keys[0], values[0]);
    ar_hash_map_insert(map, keys[1], values[1]);
    ar_hash_map_insert(map, keys[2], values[2]);

    U32 i = 0;
    for (ArHashMapIter *iter = ar_hash_map_iter_init(scratch.arena, map);
        ar_hash_map_iter_valid(iter);
        ar_hash_map_iter_next(iter)) {
        ArStr *key = ar_hash_map_iter_get_key_ptr(iter);
        U32 *value = ar_hash_map_iter_get_value_ptr(iter);

        AR_ASSERT(ar_str_match(*key, keys[i], AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(*value == values[i]);

        i++;
    }

    ar_scratch_release(&scratch);
    AR_SUCCESS();
}

ArTestResult test_hash_map(ArArena *arena) {
    ArTestState state = ar_test_begin(arena);

    AR_RUN_TEST(&state, test_hash_map_insert);
    AR_RUN_TEST(&state, test_hash_map_has);
    AR_RUN_TEST(&state, test_hash_map_get);
    AR_RUN_TEST(&state, test_hash_map_set);
    AR_RUN_TEST(&state, test_hash_map_remove);
    AR_RUN_TEST(&state, test_hash_map_get_ptr);
    AR_RUN_TEST(&state, test_hash_map_iter);

    return ar_test_end(state);
}
