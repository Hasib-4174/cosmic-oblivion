#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define ASSERT_EQ(expected, actual, msg) do { \
    if ((expected) == (actual)) { \
        printf("  PASS: %s\n", msg); \
    } else { \
        printf("  FAIL: %s (expected %d, got %d)\n", msg, (int)(expected), (int)(actual)); \
        tests_failed++; \
    } \
    tests_run++; \
} while(0)

#define ASSERT_FLOAT_EQ(expected, actual, epsilon, msg) do { \
    float _e = (expected); float _a = (actual); \
    if (((_e - _a) < epsilon && (_a - _e) < epsilon)) { \
        printf("  PASS: %s\n", msg); \
    } else { \
        printf("  FAIL: %s (expected %f, got %f)\n", msg, _e, _a); \
        tests_failed++; \
    } \
    tests_run++; \
} while(0)

#define RUN_TEST(fn) do { \
    printf("Running %s...\n", #fn); \
    fn(); \
} while(0)

#define TEST_SUITE(name) void name(void)

#define PRINT_SUMMARY() do { \
    printf("\n========================================\n"); \
    printf("Tests run: %d, Failed: %d\n", tests_run, tests_failed); \
    printf("========================================\n"); \
    return tests_failed; \
} while(0)

#endif
