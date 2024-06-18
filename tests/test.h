#ifndef TEST_H
#define TEST_H

#include "arkin_test.h"

extern ArTestResult test_core(ArArena *arena);
extern ArTestResult test_ll(ArArena *arena);
extern ArTestResult test_strings(ArArena *arena);
extern ArTestResult test_hash_map(ArArena *arena);
extern ArTestResult test_pool(ArArena *arena);

#endif
