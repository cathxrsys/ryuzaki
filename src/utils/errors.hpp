#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <utility>

namespace ryuzaki {

    enum class ErrorCode : uint8_t {
        None = 0,
        Other
    };

    struct Error {
        ErrorCode code;
        std::string message;
    };

    template <typename T>
    class Result {
    private:
        std::variant<T, ryuzaki::Error> data_;

    public:
        Result(T value)
            : data_(std::move(value)) {
        }

        Result(ryuzaki::Error error)
            : data_(std::move(error)) {
        }

        bool is_error() const noexcept {
            return std::holds_alternative<ryuzaki::Error>(data_);
        }

        bool is_ok() const noexcept {
            return !is_error();
        }

        T& value() {
            return std::get<T>(data_);
        }

        const T& value() const {
            return std::get<T>(data_);
        }

        ryuzaki::Error& error() {
            return std::get<ryuzaki::Error>(data_);
        }

        const ryuzaki::Error& error() const {
            return std::get<ryuzaki::Error>(data_);
        }
    };

}