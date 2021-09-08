#ifndef DJX_LOGGING_H
#define DJX_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

namespace DJX
{
class Logger
{
public:
	enum LogLevel
	{
		TRACE,
    	DEBUG,
    	INFO,
    	WARN,
    	ERROR,
  		FATAL,
    	NUM_LOG_LEVELS,
	};
	// 嵌套类，负责解析文件名
	class SourceFile
	{
	public:
	template<int N>
	SourceFile(const char (&arr)[N])
	: data_(arr),
      size_(N-1)
    {
      	const char* slash = strrchr(data_, '/'); // builtin function
      	if (slash)
      	{
        	data_ = slash + 1;
        	size_ -= static_cast<int>(data_ - arr);
      	}
    }
    explicit SourceFile(const char* filename)
  	: data_(filename)
    {
    	const char* slash = strrchr(filename, '/');
    	if (slash)
      	{
    	    data_ = slash + 1;
    	}
    	size_ = static_cast<int>(strlen(data_));
	}
	const char* data_;
    int size_;
	};
	// INFO日志信息
  	Logger(SourceFile file, int line);
	// WARN ERROR FATAL日志信息
	Logger(SourceFile file, int line, LogLevel level);
	// TRACE和DEBUG日志信息
  	Logger(SourceFile file, int line, LogLevel level, const char* func);
	// SYS日志信息
	Logger(SourceFile file, int line, bool toAbort);
	// 析构函数会将日志信息输出
	~Logger();
	// 返回日志流的封装类
	LogStream& stream() { return impl_.stream_; }
	static LogLevel logLevel();
	static void setLogLevel(LogLevel level);

	typedef void (*OutputFunc)(const char* msg, int len);
	typedef void (*FlushFunc)();
	static void setOutput(OutputFunc);
	static void setFlush(FlushFunc);

private:
// 嵌套类，Logger的实际实现
class Impl
{
public:
	typedef Logger::LogLevel LogLevel;
	Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
	void formatTime();
  	void finish();
	
	Timestamp time_;
	LogStream stream_;
	LogLevel level_;
	int line_;
	SourceFile basename_;
};
	Impl impl_;
};
// 全局的日志等级
extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

#define LOG_TRACE if (DJX::Logger::logLevel() <= DJX::Logger::TRACE) \
	DJX::Logger(__FILE__, __LINE__, DJX::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (DJX::Logger::logLevel() <= DJX::Logger::DEBUG) \
	DJX::Logger(__FILE__, __LINE__, DJX::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (DJX::Logger::logLevel() <= DJX::Logger::INFO) \
	DJX::Logger(__FILE__, __LINE__).stream()

#define LOG_WARN DJX::Logger(__FILE__, __LINE__, DJX::Logger::WARN).stream()
#define LOG_ERROR DJX::Logger(__FILE__, __LINE__, DJX::Logger::ERROR).stream()
#define LOG_FATAL DJX::Logger(__FILE__, __LINE__, DJX::Logger::FATAL).stream()
#define LOG_SYSERR DJX::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL DJX::Logger(__FILE__, __LINE__, true).stream()

} // namespace DJX

#endif