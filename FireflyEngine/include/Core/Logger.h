#pragma once

#include "Core.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Firefly
{
	class Logger
	{
	public:
		static void Init();

		// trace
		template<typename T>
		static void Trace(const std::string& name, const T &msg)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->trace(msg);
		}

		template<typename... Args>
		static void Trace(const std::string& name, spdlog::string_view_t fmt, const Args &... args)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->trace(fmt, args...);
		}

		// debug
		template<typename T>
		static void Debug(const std::string& name, const T &msg)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->debug(msg);
		}

		template<typename... Args>
		static void Debug(const std::string& name, spdlog::string_view_t fmt, const Args &... args)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->debug(fmt, args...);
		}

		// info
		template<typename T>
		static void Info(const std::string& name, const T &msg)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->info(msg);
		}

		template<typename... Args>
		static void Info(const std::string& name, spdlog::string_view_t fmt, const Args &... args)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->info(fmt, args...);
		}

		// warn
		template<typename T>
		static void Warn(const std::string& name, const T &msg)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->warn(msg);
		}

		template<typename... Args>
		static void Warn(const std::string& name, spdlog::string_view_t fmt, const Args &... args)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->warn(fmt, args...);
		}

		// error
		template<typename T>
		static void Error(const std::string& name, const T &msg)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->error(msg);
		}

		template<typename... Args>
		static void Error(const std::string& name, spdlog::string_view_t fmt, const Args &... args)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->error(fmt, args...);
		}

		// critical
		template<typename T>
		static void Critical(const std::string& name, const T &msg)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->critical(msg);
		}

		template<typename... Args>
		static void Critical(const std::string& name, spdlog::string_view_t fmt, const Args &... args)
		{
			std::shared_ptr<spdlog::logger> logger = GetLoggerByName(name);
			logger->critical(fmt, args...);
		}

	private:
		static std::shared_ptr<spdlog::logger> GetLoggerByName(const std::string& name);

		static std::vector<std::shared_ptr<spdlog::logger>> s_loggers;
	};
}