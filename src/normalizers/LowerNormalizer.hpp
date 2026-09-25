#pragma once

#include "DefaultNormalizer.hpp"

#include <boost/locale.hpp>
#include <locale>
#include <string>
#include <string_view>


class LowerNormalizer : public DefaultNormalizer {
public:
    std::string_view name() const override {
        return "lower";
    }

protected:
    std::string normalize_impl(std::string_view value) const override {
        static const std::locale loc = boost::locale::generator{}("en_US.UTF-8");

        return boost::locale::to_lower(std::string(value), loc);
    }
};