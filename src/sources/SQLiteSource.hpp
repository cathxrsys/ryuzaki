#pragma once

#include <string_view>
#include <filesystem>
#include <format>
#include <stdexcept>
#include <utility>

#include <boost/json.hpp>
#include "utils/files.hpp"

#include "logger.hpp"
#include "SourceTypes.hpp"
#include "ISource.hpp"

#include "normalizers/factory.hpp"

#include "sqlite3.h"

namespace json = boost::json;


class SQLiteSource : public ISource {
private:
    Source source_;
    std::filesystem::path db_path_;
    sqlite3* db_ = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
    Mapping mapping_;




    ryuzaki::Error bind_parameter(int index, std::string_view value) {
        const int rc = sqlite3_bind_text(
            stmt_,
            index,
            value.data(),
            static_cast<int>(value.size()),
            SQLITE_TRANSIENT
        );

        if (rc != SQLITE_OK) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = std::format(
                    "SQLite bind text failed: {}",
                    sqlite3_errmsg(db_)
                )
            };
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }

    ryuzaki::Error bind_parameter(int index, int value) {
        const int rc = sqlite3_bind_int(
            stmt_,
            index,
            value
        );

        if (rc != SQLITE_OK) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = sqlite3_errmsg(db_)
            };
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }

    ryuzaki::Error bind_parameter(int index, int64_t value) {
        const int rc = sqlite3_bind_int64(
            stmt_,
            index,
            value
        );

        if (rc != SQLITE_OK) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = sqlite3_errmsg(db_)
            };
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }

    ryuzaki::Error bind_parameter(int index, double value) {
        const int rc = sqlite3_bind_double(
            stmt_,
            index,
            value
        );

        if (rc != SQLITE_OK) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = sqlite3_errmsg(db_)
            };
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }

    ryuzaki::Error bind_parameter(int index, std::nullptr_t) {
        const int rc = sqlite3_bind_null(
            stmt_,
            index
        );

        if (rc != SQLITE_OK) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = sqlite3_errmsg(db_)
            };
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }



    template <typename T, typename... Args>
    ryuzaki::Error bind_parameters(int index, T&& value, Args&&... args) {
        ryuzaki::Error error = bind_parameter(index, std::forward<T>(value));

        if (error.code != ryuzaki::ErrorCode::None) {
            return error;
        }

        if constexpr (sizeof...(args) > 0) {
            return bind_parameters(
                index + 1,
                std::forward<Args>(args)...
            );
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }




    ryuzaki::Result<Mapping> parse_mapping(std::filesystem::path mapping_file) {
        ryuzaki::Result<std::string> result = ::utils::read_file_to_string(mapping_file);

        if (result.is_error()) {
            return result.error();
        }

        const std::string& content = result.value();

        json::value root;

        try {
            root = json::parse(content);
        } catch (...) {
            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = "parse_mapping: JSON parse failed"
            };
        }

        const json::object& object = root.as_object();

        Mapping mapping;

        const auto& entities = object.at("entities").as_object();

        for (const auto& [entity_name, entity_value] : entities) {
            const auto& entity_object = entity_value.as_object();
            const auto& mappings = entity_object.at("mappings").as_array();

            const auto* normalizer = entity_object.if_contains("normalizer");
            std::string normalizer_name = normalizer ? std::string(normalizer->as_string()) : "";

            auto& addresses = mapping[std::string(entity_name)];
            addresses.reserve(mappings.size());

            for (const auto& mapping_value : mappings) {
                const auto& mapping_object = mapping_value.as_object();

                addresses.push_back(Address{
                    .table = std::string(
                        mapping_object.at("table").as_string()
                    ),
                    .column = std::string(
                        mapping_object.at("column").as_string()
                    ),
                    .normalizer = normalizer_name
                });
            }
        }

        return mapping;
    }

    void clear_statement() noexcept {
        if (stmt_) {
            sqlite3_finalize(stmt_);
            stmt_ = nullptr;
        }
    }

    template <typename... Args>
    ryuzaki::Error execute(
        std::string_view sql,
        Args&&... args
    ) {
        clear_statement();

        const int rc = sqlite3_prepare_v2(
            db_,
            sql.data(),
            static_cast<int>(sql.size()),
            &stmt_,
            nullptr
        );

        if (rc != SQLITE_OK) {
            clear_statement();

            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = std::format(
                    "SQLite prepare failed: {}",
                    sqlite3_errmsg(db_)
                )
            };
        }

        ryuzaki::Error error{
            .code = ryuzaki::ErrorCode::None
        };

        int index = 1;

        (
            [&] {
                if (error.code != ryuzaki::ErrorCode::None) {
                    return;
                }

                error = bind_parameter(
                    index++,
                    std::forward<Args>(args)
                );
            }(),
            ...
        );

        if (error.code != ryuzaki::ErrorCode::None) {
            clear_statement();
            return error;
        }

        const int step_rc = sqlite3_step(stmt_);

        if (step_rc != SQLITE_ROW && step_rc != SQLITE_DONE) {
            const std::string sqlite_error = sqlite3_errmsg(db_);

            clear_statement();

            return ryuzaki::Error{
                .code = ryuzaki::ErrorCode::Other,
                .message = std::format(
                    "SQLite execution failed: {}",
                    sqlite_error
                )
            };
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None
        };
    }

    boost::json::array fetch_results_json() {
        namespace json = boost::json;

        json::array results;

        if (!stmt_) {
            return results;
        }

        const int column_count = sqlite3_column_count(stmt_);

        if (column_count == 0 || sqlite3_data_count(stmt_) == 0) {
            clear_statement();
            return results;
        }

        while (true) {
            json::object row;

            for (int i = 0; i < column_count; ++i) {
                const char* column_name =
                    sqlite3_column_name(stmt_, i);

                switch (sqlite3_column_type(stmt_, i)) {
                    case SQLITE_INTEGER:
                        row[column_name] =
                            sqlite3_column_int64(stmt_, i);
                        break;

                    case SQLITE_FLOAT:
                        row[column_name] =
                            sqlite3_column_double(stmt_, i);
                        break;

                    case SQLITE_TEXT: {
                        const auto* value =
                            sqlite3_column_text(stmt_, i);

                        row[column_name] =
                            value
                                ? reinterpret_cast<const char*>(value)
                                : "";
                        break;
                    }

                    case SQLITE_NULL:
                        row[column_name] = nullptr;
                        break;

                    case SQLITE_BLOB: {
                        const int size =
                            sqlite3_column_bytes(stmt_, i);

                        row[column_name] =
                            "[BLOB " + std::to_string(size) + " bytes]";
                        break;
                    }
                }
            }

            results.push_back(std::move(row));

            const int rc = sqlite3_step(stmt_);

            if (rc == SQLITE_DONE) {
                break;
            }

            if (rc != SQLITE_ROW) {
                break;
            }
        }

        clear_statement();

        return results;
    }
public:
    ~SQLiteSource() override {
        clear_statement();

        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    SQLiteSource(Source source) : source_(std::move(source)) {
    }

    std::string_view name() const override {
        return "sqlite";
    }

    SQLiteSource(const SQLiteSource&) = delete; // запрет на копирование
    SQLiteSource& operator=(const SQLiteSource&) = delete;

    ryuzaki::Error initialize() override {
        SPDLOG_INFO("SQLiteSource initizalize()");
        
        { // database initizalization
            try {
                db_path_ = source_.path / std::string(source_.metadata.at("filename").as_string());
            } catch (const std::exception& e) {
                return ryuzaki::Error{
                    .code = ryuzaki::ErrorCode::Other,
                    .message = e.what()
                };
            }

            int rc = sqlite3_open_v2(
                db_path_.c_str(),
                &db_,
                SQLITE_OPEN_READONLY,
                nullptr
            );

            if (rc != SQLITE_OK) {
                std::string error = sqlite3_errmsg(db_);

                sqlite3_close(db_);
                db_ = nullptr;

                return ryuzaki::Error{
                    .code = ryuzaki::ErrorCode::Other,
                    .message = std::format(
                        "Failed to open SQLite database '{}': {}",
                        db_path_.string(),
                        error
                    )
                };
            }

            SPDLOG_INFO("SQLite database opened: {}", db_path_.string());
        }

        { // mapping parsing
            SPDLOG_INFO("Mapping file parsing...");

            try {
                const auto mapping_path = source_.path / source_.metadata.at("mappingFile").as_string().c_str();
                
                ryuzaki::Result<Mapping> mapping = parse_mapping(std::filesystem::path{mapping_path});

                if(mapping.is_error()) {
                    return mapping.error();
                }

                mapping_ = std::move(mapping.value());

                SPDLOG_INFO("Mapping file parsed successfuly");
            } catch (...) {
                SPDLOG_ERROR("Failed to parse mapping file");

                return ryuzaki::Error{
                    .code = ryuzaki::ErrorCode::Other,
                    .message = std::format("Failed to parse mapping file")
                };
            }

            
        }

        return ryuzaki::Error{
            .code = ryuzaki::ErrorCode::None,
        };
    }

    SearchResult search(const SearchQuery& query) override {
        SPDLOG_DEBUG("SQLiteSource searching...");

        const auto it = mapping_.find(query.entity_name);

        SearchResult result{
            .error = ryuzaki::Error{.code=ryuzaki::ErrorCode::None},
            .data = boost::json::array(),
            .source = source_.name
        };

        if (it == mapping_.end()) {
            return result;
        }

        const std::vector<Address>& addresses = it->second;

        for(const Address& address : addresses) {

            std::unique_ptr<INormalizer> normalizer = select_normalizer(address.normalizer);
            if(normalizer == nullptr) {
                normalizer = select_normalizer_by_entity_name(query.entity_name);
            }

            std::string normalized_query = normalizer->normalize(query.query);

            const std::string sql = std::format(
                "SELECT * FROM \"{}\" WHERE \"{}\" LIKE ?",
                address.table,
                address.column
            );

            ryuzaki::Error e = execute(sql, std::format("%{}%", normalized_query));

            if (e.code == ryuzaki::ErrorCode::None) {
                boost::json::array fetched_results = fetch_results_json();

                result.data.insert(
                    result.data.end(),
                    fetched_results.begin(),
                    fetched_results.end()
                );
            }
        }

        return result;
    }

    
};