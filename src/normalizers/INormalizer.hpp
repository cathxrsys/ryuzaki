#pragma once

#include <string>

class INormalizer {
public:
    virtual ~INormalizer() = default;

    virtual std::string_view name() const = 0;

    virtual std::string normalize(std::string_view value) const = 0;
};