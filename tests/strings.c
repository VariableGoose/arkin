#include "arkin_core.h"
#include "arkin_log.h"
#include "arkin_test.h"
#include "test.h"

ArTestCaseResult test_char_helpers(void) {
    AR_ASSERT(ar_char_to_lower('a') == 'a');
    AR_ASSERT(ar_char_to_lower('A') == 'a');

    AR_ASSERT(ar_char_to_upper('a') == 'A');
    AR_ASSERT(ar_char_to_upper('A') == 'A');

    AR_SUCCESS();
}

ArTestCaseResult test_string_match(void) {
    {
        ArStr a = ar_str_lit("arkin");
        ArStr b = ar_str_lit("arkin");

        B8 match = ar_str_match(a, b, AR_STR_MATCH_FLAG_EXACT);
        AR_ASSERT_MSG(match, "Standard exact match");

        b = ar_str_lit("arkinfoobar");

        match = ar_str_match(a, b, AR_STR_MATCH_FLAG_EXACT);
        AR_ASSERT_MSG(!match, "Non-matching length");
    }

    {
        ArStr a = ar_str_lit("CaSe InSeNsItIvE");
        ArStr b = ar_str_lit("case insensitive");

        B8 match = ar_str_match(a, b, AR_STR_MATCH_FLAG_CASE_INSENSITIVE);
        AR_ASSERT_MSG(match, "Case insensitive");
    }

    {
        ArStr a = ar_str_lit("Sloppy length");
        ArStr b = ar_str_lit("Sloppy");

        B8 match = ar_str_match(a, b, AR_STR_MATCH_FLAG_SLOPPY_LENGTH);
        AR_ASSERT_MSG(match, "Sloppy length");

        match = ar_str_match(b, a, AR_STR_MATCH_FLAG_SLOPPY_LENGTH);
        AR_ASSERT_MSG(match, "Sloppy length, reverse order.");
    }

    {
        ArStr a = ar_str_lit(",.");
        ArStr b = ar_str_lit(",.");

        B8 match = ar_str_match(a, b, AR_STR_MATCH_FLAG_EXACT);
        AR_ASSERT_MSG(match, "Non alphanumeric");
    }

    {
        ArStr a = ar_str_lit("foo, bar");
        ArStr b = ar_str_lit("foo, ");

        B8 match = ar_str_match(a, b, AR_STR_MATCH_FLAG_SLOPPY_LENGTH);
        AR_ASSERT_MSG(match, "Non alphanumeric sloppy");
    }

    AR_SUCCESS();
}

ArTestCaseResult test_string_sub(void) {
    ArStr str = ar_str_lit("foobar");

    {
        ArStr expected = ar_str_lit("foo");

        ArStr foo_index = ar_str_sub(str, 0, 2);
        AR_ASSERT(ar_str_match(foo_index, expected, AR_STR_MATCH_FLAG_EXACT));

        ArStr foo_len = ar_str_sub_len(str, 0, 3);
        AR_ASSERT(ar_str_match(foo_len, expected, AR_STR_MATCH_FLAG_EXACT));
    }

    {
        ArStr expected = ar_str_lit("bar");

        ArStr bar_index = ar_str_sub(str, 3, 5);
        AR_ASSERT(ar_str_match(bar_index, expected, AR_STR_MATCH_FLAG_EXACT));

        ArStr bar_len = ar_str_sub_len(str, 3, 3);
        AR_ASSERT(ar_str_match(bar_len, expected, AR_STR_MATCH_FLAG_EXACT));
    }

    {
        ArStr expected = ar_str_lit("bar");

        ArStr sub = ar_str_sub(str, 5, 3);
        AR_ASSERT_MSG(ar_str_match(sub, expected, AR_STR_MATCH_FLAG_EXACT), "Inverse indices");
    }

    {
        ArStr chop_start = ar_str_chop_start(str, 3);
        AR_ASSERT(ar_str_match(chop_start, ar_str_lit("bar"), AR_STR_MATCH_FLAG_EXACT));

        ArStr chop_end = ar_str_chop_end(str, 3);
        AR_ASSERT(ar_str_match(chop_end, ar_str_lit("foo"), AR_STR_MATCH_FLAG_EXACT));
    }

    AR_SUCCESS();
}

ArTestCaseResult test_string_find(void) {
    ArStr str = ar_str_lit("foobarqux quxbarfoo");
    {
        U64 index = ar_str_find(str, ar_str_lit("foo"), AR_STR_MATCH_FLAG_EXACT);
        AR_ASSERT(index == 0);

        index = ar_str_find(str, ar_str_lit("foo"), AR_STR_MATCH_FLAG_LAST);
        AR_ASSERT(index == 16);

        index = ar_str_find(str, ar_str_lit("QUX"), AR_STR_MATCH_FLAG_LAST | AR_STR_MATCH_FLAG_CASE_INSENSITIVE);
        AR_ASSERT(index == 10);
    }

    {
        U64 index = ar_str_find_char(str, ' ', AR_STR_MATCH_FLAG_EXACT);
        AR_ASSERT(index == 9);

        index = ar_str_find_char(str, 'b', AR_STR_MATCH_FLAG_LAST);
        AR_ASSERT(index == 13);

        index = ar_str_find_char(str, 'Q', AR_STR_MATCH_FLAG_LAST | AR_STR_MATCH_FLAG_CASE_INSENSITIVE);
        AR_ASSERT(index == 10);

        index = ar_str_find_char(str, '1', AR_STR_MATCH_FLAG_LAST);
        AR_ASSERT(index == str.len);
    }

    ArStr str2 = ar_str_lit("foo, bar, qux");
    {
        U64 index = ar_str_find(str2, ar_str_lit(", "), AR_STR_MATCH_FLAG_EXACT);
        AR_ASSERT(index == 3);
    }

    AR_SUCCESS();
}

ArTestCaseResult test_string_split(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    {
        ArStr str = ar_str_lit("foo, bar, qux");

        ArStrList split = ar_str_split(scratch.arena, str, ar_str_lit(", "), AR_STR_MATCH_FLAG_EXACT);

        ArStrListNode *foo = split.first;
        ArStrListNode *bar = foo->next;
        ArStrListNode *qux = bar->next;

        AR_ASSERT(ar_str_match(foo->str, ar_str_lit("foo"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(ar_str_match(bar->str, ar_str_lit("bar"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(ar_str_match(qux->str, ar_str_lit("qux"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(qux->next == NULL);
    }

    {
        ArStr str = ar_str_lit("fooSPLITbarSPLITqux");

        ArStrList split = ar_str_split(scratch.arena, str, ar_str_lit("split"), AR_STR_MATCH_FLAG_CASE_INSENSITIVE);

        ArStrListNode *foo = split.first;
        ArStrListNode *bar = foo->next;
        ArStrListNode *qux = bar->next;

        AR_ASSERT(ar_str_match(foo->str, ar_str_lit("foo"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(ar_str_match(bar->str, ar_str_lit("bar"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(ar_str_match(qux->str, ar_str_lit("qux"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(qux->next == NULL);
    }

    {
        ArStr str = ar_str_lit("foo, bar, qux");

        ArStrList split = ar_str_split_char(scratch.arena, str, ' ', AR_STR_MATCH_FLAG_EXACT);

        ArStrListNode *foo = split.first;
        ArStrListNode *bar = foo->next;
        ArStrListNode *qux = bar->next;

        AR_ASSERT(ar_str_match(foo->str, ar_str_lit("foo,"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(ar_str_match(bar->str, ar_str_lit("bar,"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(ar_str_match(qux->str, ar_str_lit("qux"), AR_STR_MATCH_FLAG_EXACT));
        AR_ASSERT(qux->next == NULL);
    }


    ar_scratch_release(&scratch);

    AR_SUCCESS();
}

ArTestCaseResult test_string_list(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    {
        ArStrList list = {0};

        ar_str_list_push(scratch.arena, &list, ar_str_lit("foo"));
        ar_str_list_push(scratch.arena, &list, ar_str_lit("bar"));

        AR_ASSERT(ar_str_match(list.first->str, ar_str_lit("foo"), 0));
        AR_ASSERT(ar_str_match(list.last->str, ar_str_lit("bar"), 0));
    }

    {
        ArStrList list = {0};

        ar_str_list_push_front(scratch.arena, &list, ar_str_lit("foo"));
        ar_str_list_push_front(scratch.arena, &list, ar_str_lit("bar"));

        AR_ASSERT(ar_str_match(list.first->str, ar_str_lit("bar"), 0));
        AR_ASSERT(ar_str_match(list.last->str, ar_str_lit("foo"), 0));
    }

    {
        ArStrList list = {0};

        ar_str_list_push(scratch.arena, &list, ar_str_lit("foo"));
        ar_str_list_push(scratch.arena, &list, ar_str_lit("bar"));
        ar_str_list_push(scratch.arena, &list, ar_str_lit("qux"));
        ar_str_list_push_front(scratch.arena, &list, ar_str_lit("lorem"));

        ArStr join = ar_str_list_join(scratch.arena, list);

        AR_ASSERT(ar_str_match(join, ar_str_lit("loremfoobarqux"), 0));
    }

    ar_scratch_release(&scratch);

    AR_SUCCESS();
}

ArTestCaseResult test_string_trim(void) {
    {
        ArStr trim = ar_str_trim(ar_str_lit("   foobar     \n "));
        AR_ASSERT(ar_str_match(trim, ar_str_lit("foobar"), 0));
    }

    {
        ArStr trim = ar_str_trim_front(ar_str_lit("   foobar   \n"));
        AR_ASSERT(ar_str_match(trim, ar_str_lit("foobar   \n"), 0));
    }

    {
        ArStr trim = ar_str_trim_back(ar_str_lit("   foobar   \n"));
        AR_ASSERT(ar_str_match(trim, ar_str_lit("   foobar"), 0));
    }

    AR_SUCCESS();
}

ArTestCaseResult test_string_format(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    ArStr format = ar_str_format(scratch.arena, "foo%d", 42);
    AR_ASSERT(ar_str_match(format, ar_str_lit("foo42"), 0));

    ar_scratch_release(&scratch);

    AR_SUCCESS();
}

ArTestCaseResult test_string_copy(void) {
    ArTemp scratch = ar_scratch_get(NULL, 0);

    ArStr copy = ar_str_push_copy(scratch.arena, ar_str_lit("foobar"));
    AR_ASSERT(ar_str_match(copy, ar_str_lit("foobar"), 0));

    ar_scratch_release(&scratch);

    AR_SUCCESS();
}

ArTestResult test_strings(void) {
    ArTestState state = ar_test_begin();

    AR_RUN_TEST(&state, test_char_helpers);
    AR_RUN_TEST(&state, test_string_match);
    AR_RUN_TEST(&state, test_string_sub);
    AR_RUN_TEST(&state, test_string_find);
    AR_RUN_TEST(&state, test_string_split);
    AR_RUN_TEST(&state, test_string_list);
    AR_RUN_TEST(&state, test_string_format);
    AR_RUN_TEST(&state, test_string_copy);

    return ar_test_end(state);
}
