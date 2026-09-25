#pragma once

#include <unordered_map>
#include <string>
#include <mutex>
#include <optional>

#include "IStorage.hpp"

#include "task/task.hpp"


class RamStorage : public IStorage {
private:
    std::unordered_map<std::string, SearchTask> tasks_;
    std::mutex mutex_;
public:
    void save(const SearchTask& task) override {
        std::lock_guard lock(mutex_);
        tasks_[task.id] = task;
    }

    std::optional<SearchTask> get(const std::string& task_id) override {
        std::lock_guard lock(mutex_);

        auto i = tasks_.find(task_id);
        if (i != tasks_.end())
            return i->second;

        return std::nullopt;
    }
};