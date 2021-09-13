#ifndef DJX_LOGFILE_H
#define DJX_LOGFILE_H

#include "Mutex.h"

#include <memory>
#include <string>

namespace DJX
{

namespace FileUtil
{
class AppendFile;
} // namespace FileUtil

class LogFile : noncopyable
{
public:
	LogFile(const std::string& basename,
			off_t rollSize_,
			bool threadSafe = true,
			int flushInterval = 3
			//int checkEveryN_ = 1024
			);
	~LogFile();
	// 对外的append接口，将日志信息添加到文件的写缓冲区
	void append(const char* logline, int len);
	// 刷新缓冲区
	void flush();
	// 滚动日志文件
	bool rollFile();

private:
	// 非线程安全的append
	void append_unlocked(const char* logline, int len);

	static std::string getLogFileName(const std::string& basename, time_t* now);

	const std::string basename_;
	// 当文件写入量到达rollsize时换文件写日志
	const off_t rollSize_;
	// 写入操作的间隔时间
	const int flushInterval_;
	// append的次数
	//int count_;

	std::unique_ptr<MutexLock> mutex_;
	// 日志开始记录的时间（调整为零点）
	time_t start_;
	// 上一次滚动日志的时间
	time_t lastRoll_;
	// 上一次日志写入的时间
	time_t lastFlush_;
	// 滚动日志的时间间隔
	const static int kRollPerSeconds_ =  60*60*24;
	// 滚动日志的append间隔检查
	//int checkEveryN_;

	std::unique_ptr<FileUtil::AppendFile> file_;
};

} // namespace DJX
#endif