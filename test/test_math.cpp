// test/test_math.cpp
#include <gtest/gtest.h>
#include "math.h"

TEST(MathTest, AddPositive) {
    EXPECT_EQ(add(2, 3), 5);
}

TEST(MathTest, SubNegative) {
    EXPECT_EQ(sub(10, 7), 3);
}


