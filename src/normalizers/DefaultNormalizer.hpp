#pragma once

#include "INormalizer.hpp"

#include <string>
#include <string_view>

class DefaultNormalizer : public INormalizer {
public:
    std::string normalize(std::string_view value) const final {
        return normalize_impl(trim(value));
    }

    std::string_view name() const override {
        return "default";
    }

protected:
    virtual std::string normalize_impl(std::string_view value) const {
        return std::string(value);
    }

private:
    static std::string trim(std::string_view value) {
        const auto begin = value.find_first_not_of(" \t\n\r\f\v");

        if (begin == std::string_view::npos) {
            return {};
        }

        const auto end = value.find_last_not_of(" \t\n\r\f\v");

        return std::string(value.substr(begin, end - begin + 1));
    }
};