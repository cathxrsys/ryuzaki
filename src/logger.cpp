#include "logger.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace logging {

void init() {
    spdlog::init_thread_pool(8192, 1);
    
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("ryuzaki.log", true);
    
    auto async_logger = std::make_shared<spdlog::async_logger>(
        "ryuzaki_logger", 
        // spdlog::sinks_init_list{console_sink, file_sink}, 
        spdlog::sinks_init_list{console_sink}, 
        spdlog::thread_pool(), 
        spdlog::async_overflow_policy::block
    );
    
    spdlog::register_logger(async_logger);
    spdlog::set_default_logger(async_logger);
    
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
    spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
    
    spdlog::info("spdlog init successfully");
}

void shutdown() {
    spdlog::shutdown();
}

} // namespace logging