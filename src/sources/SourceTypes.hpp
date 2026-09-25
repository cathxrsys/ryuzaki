#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <boost/json.hpp>

#include "utils/errors.hpp"

struct SearchQuery {
    std::string entity_name;
    std::string query;
};

struct SearchResult {
    ryuzaki::Error error;
    boost::json::array data;
    std::string source;
};

struct Source {
    std::string name;
    std::filesystem::path path;
    std::string type;
    boost::json::object metadata;
};

struct Address {
    std::string table;
    std::string column;
    std::string normalizer;
};

using Mapping = std::unordered_map<
    std::string,
    std::vector<Address>
>;