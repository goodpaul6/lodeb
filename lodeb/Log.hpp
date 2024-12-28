#pragma once

#include <format>
#include <iostream>
#include <iterator>

namespace lodeb {
    enum class LogLevel {
        Debug,
        Info,
        Error,
    };

    template <typename... Args>
    void Log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
        std::ostream_iterator<char> stream_iter{std::cout};

        if(level == LogLevel::Debug) {
            std::cout << "[DEBUG] ";
        } else if(level == LogLevel::Info) {
            std::cout << "[INFO]  ";
        } else if(level == LogLevel::Error) {
            std::cerr << "[ERROR] ";
            stream_iter = {std::cerr};
        }

        std::format_to(stream_iter, fmt, std::forward<Args>(args)...);
        stream_iter = '\n';
    }

    template <typename... Args>
    void LogDebug(std::format_string<Args...> fmt, Args&&... args) {
       Log(LogLevel::Debug, fmt, std::forward<Args>(args)...); 
    }

    template <typename... Args>
    void LogInfo(std::format_string<Args...> fmt, Args&&... args) {
       Log(LogLevel::Info, fmt, std::forward<Args>(args)...); 
    }

    template <typename... Args>
    void LogError(std::format_string<Args...> fmt, Args&&... args) {
       Log(LogLevel::Error, fmt, std::forward<Args>(args)...); 
    }
}
