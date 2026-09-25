#pragma once

#include <unordered_map>
#include <string_view>


namespace constants {
    inline const std::unordered_map<std::string_view, std::string_view> C_NORMALIZERS = {
        {"default", "lower"},
        {"name", "lower"},
        {"phone", "onlynumeric"},
        {"email", "lower"},
        {"address", "lower"},
        {"card_number", "onlynumeric"},
    };
}