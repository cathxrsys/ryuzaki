#pragma once

#include "DefaultNormalizer.hpp"

#include <string>
#include <string_view>
#include <cctype>


class OnlyNumericNormalizer : public DefaultNormalizer {
public:
    std::string_view name() const override {
        return "onlynumeric";
    }

    std::string normalize_impl(std::string_view value) const override {
        std::string result;
        result.reserve(value.size());

        for (const unsigned char c : value) {
            if (std::isdigit(c)) {
                result += static_cast<char>(c);
            }
        }

        return result;
    }
};