#include "arkin_core.h"
#include "arkin_test.h"
#include "arkin_log.h"

typedef struct DLLNode DLLNode;
struct DLLNode {
    DLLNode *next;
    DLLNode *prev;

    U32 value;
};

static B8 dll_check_connections(DLLNode *node, DLLNode *next, DLLNode *prev) {
    return node->next == next &&
        node->prev == prev &&
        prev != NULL ? prev->next == node : true &&
        next != NULL ? next->prev == node : true;
}

ArTestCaseResult test_dll_insert(void) {
    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        ar_dll_insert(first, last, &nodes[0], last);
        AR_ASSERT_MSG(dll_check_connections(first, NULL, NULL) && first == last && first == &nodes[0], "Initial insert");

        ar_dll_insert(first, last, &nodes[2], last);
        AR_ASSERT_MSG(
                first == &nodes[0] &&
                last == &nodes[2] &&
                dll_check_connections(&nodes[0], &nodes[2], NULL) &&
                dll_check_connections(&nodes[2], NULL, &nodes[0]),
                "Insert back, single node");

        ar_dll_insert(first, last, &nodes[1], first);
        AR_ASSERT_MSG(
                first == &nodes[0] &&
                last == &nodes[2] &&
                dll_check_connections(&nodes[0], &nodes[1], NULL) &&
                dll_check_connections(&nodes[1], &nodes[2], &nodes[0]) &&
                dll_check_connections(&nodes[2], NULL, &nodes[1]),
                "Insert middle");

        ar_dll_insert(first, last, &nodes[3], (DLLNode *) 0);
        AR_ASSERT_MSG(
                first == &nodes[3] &&
                last == &nodes[2] &&
                dll_check_connections(&nodes[0], &nodes[1], &nodes[3]) &&
                dll_check_connections(&nodes[1], &nodes[2], &nodes[0]) &&
                dll_check_connections(&nodes[2], NULL, &nodes[1]) &&
                dll_check_connections(&nodes[3], &nodes[0], NULL),
                "Push front");
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_dll_push_back(first, last, &nodes[i]);
        }

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            DLLNode *next = i < ar_arrlen(nodes) - 1 ? &nodes[i + 1] : NULL;
            DLLNode *prev = i > 0 ? &nodes[i - 1] : NULL;

            AR_ASSERT_MSG(dll_check_connections(&nodes[i], next, prev), "Push back");
        }
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_dll_push_front(first, last, &nodes[i]);
        }

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            DLLNode *next = i > 0 ? &nodes[i - 1] : NULL;
            DLLNode *prev = i < ar_arrlen(nodes) - 1 ? &nodes[i + 1] : NULL;

            AR_ASSERT_MSG(dll_check_connections(&nodes[i], next, prev), "Push back");
        }
    }

    AR_SUCCESS();
}

ArTestCaseResult test_dll_remove(void) {
    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        ar_dll_insert(first, last, &nodes[0], last);
        ar_dll_remove(first, last, &nodes[0]);

        AR_ASSERT_MSG(first == last && first == NULL, "Remove node from single node dll");
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        ar_dll_push_back(first, last, &nodes[0]);
        ar_dll_push_back(first, last, &nodes[1]);
        ar_dll_remove(first, last, &nodes[0]);

        AR_ASSERT_MSG(first == last && first == &nodes[1] && dll_check_connections(&nodes[1], NULL, NULL), "Remove node from double node dll");
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        ar_dll_push_back(first, last, &nodes[0]);
        ar_dll_push_back(first, last, &nodes[1]);
        ar_dll_push_back(first, last, &nodes[2]);
        ar_dll_remove(first, last, &nodes[1]);

        AR_ASSERT_MSG(
                first == &nodes[0] &&
                last == &nodes[2] &&
                dll_check_connections(&nodes[0], &nodes[2], NULL) &&
                dll_check_connections(&nodes[2], NULL, &nodes[0]),
                "Remove node middle");
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_dll_push_back(first, last, &nodes[i]);
        }

        for (U32 i = 1; i < ar_arrlen(nodes); i += 2) {
            ar_dll_remove(first, last, &nodes[i]);
        }

        for (U32 i = 0; i < ar_arrlen(nodes); i += 2) {
            DLLNode *next = i < ar_arrlen(nodes) - 2 ? &nodes[i + 2] : NULL;
            DLLNode *prev = i > 1 ? &nodes[i - 2] : NULL;

            AR_ASSERT_MSG(dll_check_connections(&nodes[i], next, prev), "Remove every other");
        }
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_dll_push_back(first, last, &nodes[i]);
        }

        ar_dll_pop_front(first, last);

        for (U32 i = 1; i < ar_arrlen(nodes); i++) {
            DLLNode *next = i < ar_arrlen(nodes) - 1 ? &nodes[i + 1] : NULL;
            DLLNode *prev = i > 1 ? &nodes[i - 1] : NULL;

            AR_ASSERT_MSG(dll_check_connections(&nodes[i], next, prev), "Pop front");
        }
        AR_ASSERT_MSG(first == &nodes[1], "Pop front");
    }

    {
        DLLNode nodes[32] = {0};
        DLLNode *first = NULL;
        DLLNode *last = NULL;

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_dll_push_back(first, last, &nodes[i]);
        }

        ar_dll_pop_back(first, last);

        for (U32 i = 0; i < ar_arrlen(nodes) - 1; i++) {
            DLLNode *next = i < ar_arrlen(nodes) - 2 ? &nodes[i + 1] : NULL;
            DLLNode *prev = i > 0 ? &nodes[i - 1] : NULL;

            AR_ASSERT_MSG(dll_check_connections(&nodes[i], next, prev), "Pop back");
        }
        AR_ASSERT_MSG(last == &nodes[ar_arrlen(nodes) - 2], "Pop back");
    }

    AR_SUCCESS();
}

typedef struct SLLNode SLLNode;
struct SLLNode {
    SLLNode *next;
    U32 value;
};

ArTestCaseResult test_sll_queue_push(void) {
    {
        SLLNode *first = NULL;
        SLLNode *last = NULL;
        SLLNode nodes[32] = {0};

        ar_sll_queue_push(first, last, &nodes[0]);
        AR_ASSERT_MSG(first == last, "Push into empty queue");
        AR_ASSERT_MSG(nodes[0].next == NULL, "Push into empty queue");
    }

    {
        SLLNode *first = NULL;
        SLLNode *last = NULL;
        SLLNode nodes[32] = {0};

        ar_sll_queue_push(first, last, &nodes[0]);
        ar_sll_queue_push(first, last, &nodes[1]);

        AR_ASSERT_MSG(first == &nodes[0], "Queue push first");
        AR_ASSERT_MSG(last == &nodes[1], "Queue push last");
        AR_ASSERT_MSG(first->next == &nodes[1], "Queue push first->next");
    }

    {
        SLLNode *first = NULL;
        SLLNode *last = NULL;
        SLLNode nodes[32] = {0};

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_sll_queue_push(first, last, &nodes[i]);
        }

        AR_ASSERT_MSG(first == &nodes[0], "Queue push first");
        AR_ASSERT_MSG(last == &nodes[31], "Queue push last");
        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            SLLNode *next = i < ar_arrlen(nodes) - 1 ? &nodes[i + 1] : NULL;
            AR_ASSERT(nodes[i].next == next);
        }
    }

    AR_SUCCESS();
}

ArTestCaseResult test_sll_queue_pop(void) {
    {
        SLLNode *first = NULL;
        SLLNode *last = NULL;
        SLLNode nodes[32] = {0};

        ar_sll_queue_pop(first, last);
        AR_ASSERT_MSG(first == last && first == NULL, "Pop empty queue");
    }

    {
        SLLNode *first = NULL;
        SLLNode *last = NULL;
        SLLNode nodes[32] = {0};

        ar_sll_queue_push(first, last, &nodes[0]);
        ar_sll_queue_pop(first, last);
        AR_ASSERT_MSG(first == last && first == NULL, "Pop single element queue");
    }

    {
        SLLNode *first = NULL;
        SLLNode *last = NULL;
        SLLNode nodes[32] = {0};

        for (U32 i = 0; i < ar_arrlen(nodes); i++) {
            ar_sll_queue_push(first, last, &nodes[i]);
        }

        ar_sll_queue_pop(first, last);

        AR_ASSERT_MSG(first == &nodes[1], "Queue pop first");
        AR_ASSERT_MSG(last == &nodes[31], "Queue pop last");
        for (U32 i = 1; i < ar_arrlen(nodes); i++) {
            SLLNode *next = i < ar_arrlen(nodes) - 1 ? &nodes[i + 1] : NULL;
            AR_ASSERT(nodes[i].next == next);
        }
    }

    AR_SUCCESS();
}

ArTestResult test_ll(ArArena *arena) {
    ArTestState state = ar_test_begin(arena);

    AR_RUN_TEST(&state, test_dll_insert);
    AR_RUN_TEST(&state, test_dll_remove);
    AR_RUN_TEST(&state, test_sll_queue_push);
    AR_RUN_TEST(&state, test_sll_queue_pop);

    return ar_test_end(state);
}
