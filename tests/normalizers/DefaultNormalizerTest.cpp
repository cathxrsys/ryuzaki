#include <gtest/gtest.h>

#include "normalizers/DefaultNormalizer.hpp"


TEST(DefaultNormalizerTest, TrimTest) {
    DefaultNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("   John Doe      "), "John Doe");
    EXPECT_EQ(normalizer.normalize("   John Doe    \n\t\n  "), "John Doe");
    EXPECT_EQ(normalizer.normalize("\t\t\t\n\n\nJohn Doe\n\n\n\n\t\t\t\t"), "John Doe");
}
