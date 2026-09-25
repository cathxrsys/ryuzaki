#include <gtest/gtest.h>

#include "normalizers/OnlyNumericNormalizer.hpp"


TEST(OnlyNumericNormalizerTest, RemovesFormattingCharacters) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("+7 (900) 155 50-04"), "79001555004");
    EXPECT_EQ(normalizer.normalize("7 (900) 155 50-04"), "79001555004");
    EXPECT_EQ(normalizer.normalize("+79001555004"), "79001555004");
    EXPECT_EQ(normalizer.normalize("1234 1234 1234 1234"), "1234123412341234");
}

TEST(OnlyNumericNormalizerTest, HandlesSpaces) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("   "), "");
}

TEST(OnlyNumericNormalizerTest, HandlesWhitespaceCharacters) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("7900\n155\t50 04"), "79001555004");
}

TEST(OnlyNumericNormalizerTest, HandlesNormalized) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("79001555004"), "79001555004");
    EXPECT_EQ(normalizer.normalize("1234123412341234"), "1234123412341234");
}

TEST(OnlyNumericNormalizerTest, HandlesEmptyInput) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize(""), "");
}

TEST(OnlyNumericNormalizerTest, HandlesInputWithoutDigits) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("+-() "), "");
    EXPECT_EQ(normalizer.normalize("hello world"), "");
}

TEST(OnlyNumericNormalizerTest, RemovesVariousSeparators) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("7900.155.50.04"), "79001555004");
    EXPECT_EQ(normalizer.normalize("7900/155/50/04"), "79001555004");
    EXPECT_EQ(normalizer.normalize("7900_155_50_04"), "79001555004");
}

TEST(OnlyNumericNormalizerTest, RemovesTextAroundPhoneNumber) {
    OnlyNumericNormalizer normalizer;

    EXPECT_EQ(normalizer.normalize("телефон: +7 (900) 155-50-04"), "79001555004");
    EXPECT_EQ(normalizer.normalize("Phone: +7 (900) 155-50-04"), "79001555004");
    EXPECT_EQ(normalizer.normalize("Card number: 1234 1234 1234 1234"), "1234123412341234");
}