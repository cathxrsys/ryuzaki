#pragma once

#include <string_view>
#include <string>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <boost/json.hpp>

#include "logger.hpp"

#include "utils/files.hpp"
#include "utils/errors.hpp"


inline constexpr std::string_view DEFAULT_CONFIG_FILENAME = "config.json";

class Config {
    private:
        boost::json::value json_; // оставил потому что потом вместо обращения к публичным полям буду использовать get<>()
        
    public:
        // DEFAULT VALUES
        std::string ip = "0.0.0.0";
        uint16_t port = 10511;

        Config() {
            if (!std::filesystem::exists(DEFAULT_CONFIG_FILENAME)) {
                SPDLOG_INFO("Config `{}` not found", DEFAULT_CONFIG_FILENAME);
                return;
            }
            const ryuzaki::Result result = ::utils::read_file_to_string(DEFAULT_CONFIG_FILENAME);
            if (result.is_ok()) {
                try {
                    json_ = boost::json::parse(result.value());
                    const auto& object = json_.as_object();

                    std::string new_ip = std::string(object.at("ip").as_string());

                    int64_t port_raw = object.at("port").as_int64();
                    if (port_raw <= 0 || port_raw > 65535) {
                        throw std::runtime_error("Port is out of range, must be in [1..65535]");
                    }
                    uint16_t new_port = static_cast<uint16_t>(port_raw);

                    ip = std::move(new_ip);
                    port = new_port;

                    SPDLOG_INFO("Loaded config as `{}`", DEFAULT_CONFIG_FILENAME);
                } catch (const std::exception& e) {
                    SPDLOG_ERROR("Config `{}` found but not loaded: `{}`", DEFAULT_CONFIG_FILENAME, e.what());
                }              
            } else {
                SPDLOG_ERROR("Config `{}` file load error", DEFAULT_CONFIG_FILENAME);
            }
        }
};