#include <drogon/drogon.h>
#include <iostream>
#include <thread>
#include <memory>
#include <filesystem>
#include <cstdlib>

#include "logger.hpp"

#include "constants/paths.hpp"
#include "task/TaskManager.hpp"
#include "storage/RamStorage.hpp"
#include "sources/SourceManager.hpp"
#include "config/config.hpp"

#include "globals.hpp"


int main() {
    logging::init();

    Config config;

    const unsigned int cores_count = std::thread::hardware_concurrency();

    std::shared_ptr<RamStorage> storage = std::make_shared<RamStorage>();

    std::filesystem::path data_path = std::filesystem::current_path() / constants::C_DATA_PATH;

    if (!std::filesystem::is_directory(data_path)) {
        SPDLOG_ERROR("Data path `{}` does not exists, exiting...", data_path.c_str());
        return EXIT_FAILURE;
    }

    std::shared_ptr<SourceManager> source_manager = std::make_shared<SourceManager>(data_path);

    g_task_manager = std::make_shared<TaskManager>(storage, source_manager, cores_count);

    drogon::app().setThreadNum(cores_count);
    drogon::app().addListener(config.ip, config.port);
    drogon::app().setLogLevel(trantor::Logger::kInfo);

    spdlog::info("Server started on http://{}:{}", config.ip, config.port);

    drogon::app().run();

    g_task_manager.reset();
    storage.reset();
    source_manager.reset();

    logging::shutdown();

    return EXIT_SUCCESS;
}