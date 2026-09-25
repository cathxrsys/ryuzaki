#include <gtest/gtest.h>

#include "normalizers/LowerNormalizer.hpp"


TEST(LowerNormalizerTest, MixedCaseTest) {
    LowerNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("John Doe"), "john doe");
}

TEST(LowerNormalizerTest, OnlyLowerTest) {
    LowerNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("john doe"), "john doe");
}

TEST(LowerNormalizerTest, OnlyUpperTest) {
    LowerNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("JOHN DOE"), "john doe");
}

TEST(LowerNormalizerTest, RussianTest) {
    LowerNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("Иван Иванов"), "иван иванов");
}