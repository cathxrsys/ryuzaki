#pragma once

#include <fstream>
#include <string>
#include <filesystem>

#include "utils/errors.hpp"


namespace utils {

    [[nodiscard]] inline ryuzaki::Result<std::string> read_file_to_string(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);

        if (!file.is_open()) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = "Failed to open file: " + path.string()
            };
        }

        std::string content{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        if (file.bad()) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = "Failed to read file: " + path.string()
            };
        }

        return content;
    }

}