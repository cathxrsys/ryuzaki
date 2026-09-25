#pragma once

#include <string>

#include "sources/SourceTypes.hpp"
#include "utils/errors.hpp"

struct TaskResult {
    ryuzaki::Error error;
    std::vector<SearchResult> data;
};

enum class TaskStatus {
    Pending,
    InProgress,
    Completed,
    Failed
};

struct SearchTask {
    std::string id;
    SearchQuery query;
    TaskStatus status;
    TaskResult result;
};