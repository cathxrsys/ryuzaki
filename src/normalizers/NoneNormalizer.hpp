#pragma once

#include "INormalizer.hpp"

#include <string>
#include <string_view>

class NoneNormalizer : public INormalizer {
public:
    std::string normalize(std::string_view value) const final {
        return normalize_impl(value);
    }

    std::string_view name() const override {
        return "none";
    }

protected:
    virtual std::string normalize_impl(std::string_view value) const {
        return std::string(value);
    }
};