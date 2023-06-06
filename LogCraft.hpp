#pragma once

#include <fstream>
#include <memory>
#include <string>
#include <time.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_map>

#ifdef _WIN32
#define LocalTime(_tm, ptime) localtime_s(_tm, ptime)
#elif __linux__ || ANDROID || __arm__
#define LocalTime(_tm, ptime) *_tm = *localtime(ptime)
#endif

enum class LogCraftLevel {
	_INFO,
	_ERROR
};

class LogCraft {
public:

	static inline std::string getNowTimeStamp()
	{
		std::ostringstream oss;
		auto t = std::time(nullptr);
		tm _tm;
		LocalTime(&_tm, &t);

		oss << std::put_time(&_tm, "%Y-%m-%d_%H-%M-%S");
		return oss.str();
	}

	static inline std::string genLogName()
	{
		return "Log_" + getNowTimeStamp() + ".txt";
	}

	inline LogCraft(const char* basePath)
		: mBasePath(basePath)
		, mLevel(LogCraftLevel::INFO)
	{
		mLogName = genLogName();

		mStream = std::make_unique<std::ofstream>(mBasePath + mLogName);

		if (mStream->is_open() == false)
			throw "unable to open a file";
	}

	inline std::string getPrefix()
	{
		std::string prefix = getNowTimeStamp();

		switch (mLevel)
		{
		case LogCraftLevel::_INFO: prefix += " [INFO]"; break;
		case LogCraftLevel::_ERROR: prefix += " [ERROR]"; break;
		}

		prefix += ": ";

		return prefix;
	}

	inline void Log (const std::string& str)
	{
		*(mStream.get()) << getPrefix() << str << std::endl;
	}

	inline bool Save()
	{
		mStream->flush();

		return true;
	}

	static inline LogCraft* getInstance(const char* id, const char* basePath = "./")
	{
		static std::unordered_map<std::string, std::unique_ptr<LogCraft>> instances;

		if (instances.find(id) != instances.end())
			return instances[id].get();

		try {
			instances[id] = std::make_unique<LogCraft>(basePath);
		}
		catch (...)
		{
			instances.erase(id);

			return nullptr;
		}

		return instances[id].get();
	}

	inline void setLogLevel(LogCraftLevel level)
	{
		mLevel = level;
	}

private:
	std::unique_ptr<std::ofstream> mStream;
	std::string mBasePath;
	std::string mLogName;
	LogCraftLevel mLevel;

};

#define CHECK_LOG_GLOB() (LogCraft::getInstance("Global") != nullptr)
#define CHECK_LOG_GLOB_WPATH(path) LogCraft::getInstance("Global", path)

#define LOG_GLOB_PTR() LogCraft::getInstance("Global")
#define LOG_GLOB_PRINT(x) (LOG_GLOB_PTR()->Log(x))

#define LOG_GLOB_PRINT_LVL(x, errLvl) \
LOG_GLOB_PTR()->setLogLevel(errLvl);  \
LOG_GLOB_PRINT(x);

#define LOG_GLOB_PRINT_ERR(x) LOG_GLOB_PRINT_LVL(x,LogCraftLevel::_ERROR)
#define LOG_GLOB_PRINT_INFO(x) LOG_GLOB_PRINT_LVL(x,LogCraftLevel::_INFO)