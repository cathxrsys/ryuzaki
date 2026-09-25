#include <gtest/gtest.h>

#include "normalizers/NoneNormalizer.hpp"


TEST(NoneNormalizerTest, NoneTest) {
    NoneNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("   John Doe      "), "   John Doe      ");
}