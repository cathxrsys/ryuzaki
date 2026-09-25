#pragma once

#include <optional>
#include <string>

#include "task/task.hpp"

class IStorage {
public:
    virtual void save(const SearchTask& task) = 0;
    virtual std::optional<SearchTask> get(const std::string& task_id) = 0;
};