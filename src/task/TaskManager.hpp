#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <thread>
#include <queue>
#include <optional>
#include <mutex>

#include "logger.hpp"
#include "utils/uuid.hpp"

#include "storage/IStorage.hpp"
#include "task/task.hpp"
#include "sources/SourceTypes.hpp"
#include "sources/SQLiteSource.hpp"
#include "sources/SourceManager.hpp"


class TaskManager {
private:
    std::shared_ptr<IStorage> storage_;
    std::shared_ptr<SourceManager> source_manager_;
    std::vector<std::thread> workers_;
    std::queue<SearchTask> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_ = false;

    void worker_loop() {

        std::vector<std::unique_ptr<ISource>> sources;
        for (const Source& src : source_manager_->all()) {
            std::unique_ptr<ISource> s;

            if (src.type == "sqlite") {
                s = std::make_unique<SQLiteSource>(src);
            }

            if (!s) continue;

            ryuzaki::Error err = s->initialize();
            if (err.code != ryuzaki::ErrorCode::None) {
                SPDLOG_WARN("Source '{}' init failed: {}", src.name, err.message);
                continue;
            }
            sources.push_back(std::move(s));
        }

        while(true) {
            SearchTask task;
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this] { return stop_ || !queue_.empty(); });
                if (stop_ && queue_.empty()) return;
                task = queue_.front();
                queue_.pop();
            }
            task.status = TaskStatus::InProgress;
            storage_->save(task);

            SPDLOG_DEBUG("Worker processing task: {}", task.id);

            for (const auto& source : sources) {
                SearchResult result = source->search(task.query);

                SPDLOG_DEBUG("Search result: {}", boost::json::serialize(result.data));

                if (result.error.code != ryuzaki::ErrorCode::None) {
                    SPDLOG_WARN("Search error: {}", result.error.message);
                    continue;
                }

                task.result.data.push_back(result);
            }

            SPDLOG_DEBUG("Task done: {}", task.id);

            task.status = TaskStatus::Completed;
            storage_->save(task);
        }
    }
public:
    TaskManager(std::shared_ptr<IStorage> storage,
                std::shared_ptr<SourceManager> source_manager,
                size_t workers_count = 4) :
                
                storage_(std::move(storage)),
                source_manager_(std::move(source_manager)) {
        for(int i=0; i<workers_count; i++) {
            workers_.emplace_back(&TaskManager::worker_loop, this);
        }
        SPDLOG_INFO("TaskManager started with {} workers", workers_count);
    }
    ~TaskManager() {
        stop_.store(true);
        cv_.notify_all();

        for (std::thread& t : workers_) {
            if (t.joinable()) t.join();
        }

        SPDLOG_INFO("TaskManager stopped");
    }
    std::string create_task(SearchQuery query) {
        SearchTask task;

        task.id = ::utils::generate_uuid();
        task.query = std::move(query);
        task.status = TaskStatus::Pending;
        
        storage_->save(task);

        {
            std::lock_guard lock(mutex_);
            queue_.push(task);
        }
        cv_.notify_one();

        SPDLOG_DEBUG("Task created: {}", task.id);

        return task.id;
    }
    std::optional<SearchTask> get_task(const std::string& task_id) {
        return storage_->get(task_id);
    }
};