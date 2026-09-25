#pragma once

#include <string_view>
#include "SourceTypes.hpp"

#include "utils/errors.hpp"

class ISource {
public:
    virtual ~ISource() = default;

    virtual std::string_view name() const = 0;

    virtual ryuzaki::Error initialize() {
        // Default implementation does nothing
        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    };

    virtual SearchResult search(const SearchQuery& query) = 0;
};