#include "pch.h"
#include "Core/Logger.h"

namespace Firefly
{
    std::vector<std::shared_ptr<spdlog::logger>> Logger::s_loggers;

    void Logger::Init()
    {
        spdlog::set_pattern("%^[%T] %-8l (%n): %v%$");
    }

    std::shared_ptr<spdlog::logger> Logger::GetLoggerByName(const std::string& name)
    {
        std::shared_ptr<spdlog::logger> newLogger = nullptr;
        for (auto logger : s_loggers)
        {
            if (logger->name() == name)
            {
                newLogger = logger;
                break;
            }
        }

        if (!newLogger)
        {
            newLogger = spdlog::stdout_color_mt(name);
            newLogger->set_level(spdlog::level::trace);
            s_loggers.push_back(newLogger);
        }

        return newLogger;
    }
}