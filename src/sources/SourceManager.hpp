#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "logger.hpp"

#include <boost/json.hpp>

#include "constants/paths.hpp"
#include "SourceTypes.hpp"


class SourceManager {
private:
    std::filesystem::path root_path_;
    std::vector<Source> sources_;
public:
    SourceManager(std::filesystem::path data_path) : root_path_(data_path) {
        SPDLOG_DEBUG("Sources scanning...");
        for(const auto& entry : std::filesystem::directory_iterator(root_path_)) {
            if(!entry.is_directory()) continue;

            const auto config_path = entry.path() / constants::C_DEFAULT_CONFIG_FILENAME;

            if (!std::filesystem::exists(config_path)) continue;
            std::ifstream file(config_path);
            if (!file.is_open()) continue;
            std::stringstream buffer;
            buffer << file.rdbuf();

            try {
                const auto json = boost::json::parse(buffer.str());
                const auto& object = json.as_object();

                Source source {
                    .name = entry.path().stem().string(),
                    .path = entry.path(),
                    .type = std::string(object.at("type").as_string()),
                    .metadata = object.at("metadata").as_object(),
                };
                sources_.push_back(source);

                SPDLOG_DEBUG("Found valid source [{}] type `{}` | {}", source.name, source.type, source.path.string());
            } catch(...) {
                continue;
            }

        }
    }

    [[nodiscard]] const std::vector<Source>& all() const {
        return sources_;
    }
};