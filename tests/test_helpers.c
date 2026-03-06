#include "test_framework.h"
#include "../include/helpers.h"
#include "../include/constants.h"

TEST_SUITE(test_clampf) {
    ASSERT_EQ(5.0f, Clampf(5.0f, 0.0f, 10.0f), "Value in range returns same");
    ASSERT_EQ(0.0f, Clampf(-5.0f, 0.0f, 10.0f), "Value below range returns min");
    ASSERT_EQ(10.0f, Clampf(15.0f, 0.0f, 10.0f), "Value above range returns max");
    ASSERT_EQ(5.0f, Clampf(5.0f, 5.0f, 5.0f), "Equal bounds returns bound");
}

TEST_SUITE(test_rf) {
    float r1 = Rf(0.0f, 10.0f);
    ASSERT_EQ(1, (r1 >= 0.0f && r1 <= 10.0f), "Rf returns value in range");
    
    int in_range = 0;
    for (int i = 0; i < 1000; i++) {
        float r = Rf(0.0f, 1.0f);
        if (r >= 0.0f && r <= 1.0f) in_range++;
    }
    ASSERT_EQ(1000, in_range, "Rf always returns value in range (1000 samples)");
}

TEST_SUITE(test_calpha) {
    Color c = {255, 128, 64, 255};
    Color ca = CAlpha(c, 128);
    ASSERT_EQ(128, ca.a, "CAlpha sets alpha component");
    ASSERT_EQ(255, ca.r, "CAlpha preserves red");
    ASSERT_EQ(128, ca.g, "CAlpha preserves green");
    ASSERT_EQ(64, ca.b, "CAlpha preserves blue");
}

TEST_SUITE(test_clerp) {
    Color a = {0, 0, 0, 255};
    Color b = {255, 255, 255, 255};
    Color mid = CLerp(a, b, 0.5f);
    ASSERT_EQ(127, mid.r, "CLerp interpolates red at 0.5");
    ASSERT_EQ(127, mid.g, "CLerp interpolates green at 0.5");
    ASSERT_EQ(127, mid.b, "CLerp interpolates blue at 0.5");
    
    Color at_zero = CLerp(a, b, 0.0f);
    ASSERT_EQ(0, at_zero.r, "CLerp at t=0 returns a");
    
    Color at_one = CLerp(a, b, 1.0f);
    ASSERT_EQ(255, at_one.r, "CLerp at t=1 returns b");
}

int main(void) {
    printf("========================================\n");
    printf("Running Helper Function Tests\n");
    printf("========================================\n\n");
    
    RUN_TEST(test_clampf);
    RUN_TEST(test_rf);
    RUN_TEST(test_calpha);
    RUN_TEST(test_clerp);
    
    PRINT_SUMMARY();
}
