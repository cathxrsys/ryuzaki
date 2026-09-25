#pragma once

#include <spdlog/spdlog.h>

namespace logging {
    // Инициализация асинхронного логгера
    void init();
    
    // Корректное завершение (сброс очереди на диск)
    void shutdown();
}