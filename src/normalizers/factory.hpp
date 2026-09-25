#pragma once

#include <memory>
#include <string_view>
#include <string>

#include "logger.hpp"

#include "constants/normalizers.hpp"
#include "INormalizer.hpp"
#include "DefaultNormalizer.hpp"
#include "LowerNormalizer.hpp"
#include "OnlyNumericNormalizer.hpp"
#include "NoneNormalizer.hpp"


[[nodiscard]] inline std::unique_ptr<INormalizer> select_normalizer(std::string_view normalizer_name) {
    if (normalizer_name == "") {
        SPDLOG_DEBUG("Not found .normalizer field");
        return nullptr;
    }

    if (normalizer_name == "lower") {
        SPDLOG_DEBUG("Selected `lower` normalizer for `{}` normalizer_name", normalizer_name);
        return std::make_unique<LowerNormalizer>();
    }
    if (normalizer_name == "onlynumeric") {
        SPDLOG_DEBUG("Selected `phone` normalizer for `{}` normalizer_name", normalizer_name);
        return std::make_unique<OnlyNumericNormalizer>();
    }
    if (normalizer_name == "none") {
        SPDLOG_DEBUG("Selected `none` normalizer for `{}` normalizer_name", normalizer_name);
        return std::make_unique<NoneNormalizer>();
    }

    SPDLOG_DEBUG("Selected `default` normalizer for `{}` normalizer_name", normalizer_name);

    return std::make_unique<DefaultNormalizer>();
}

[[nodiscard]] inline std::unique_ptr<INormalizer> select_normalizer_by_entity_name(std::string_view entity_name) {
    const auto it = constants::C_NORMALIZERS.find(entity_name);

    if (it == constants::C_NORMALIZERS.end()) {
        SPDLOG_DEBUG("Selected `default` normalizer for `{}` entity", entity_name);
        return std::make_unique<DefaultNormalizer>();
    }

    const std::string_view& normalizer_name = it->second;

    return select_normalizer(normalizer_name);
}