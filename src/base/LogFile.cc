#include "LogFile.h"

#include "FileUtil.h"
#include "CurrentThread.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using std::string;

namespace DJX
{
LogFile::LogFile(const std::string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    mutex_(threadSafe ? new MutexLock : NULL),
    start_(0),
    lastRoll_(0),
    lastFlush_(0)
{
	rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_);
		append_unlocked(logline, len);
	}
	else
	{
		append_unlocked(logline, len);
	}
}

void LogFile::flush()
{
	if (mutex_)
	{
		MutexLockGuard lock(*mutex_);
    	file_->flush();
  	}
  	else
  	{
    	file_->flush();
	}
}

void LogFile::append_unlocked(const char* logline, int len)
{
	// 将数据写入到文件的写缓冲区
	file_->append(logline, len);
	// 如果写入数据大小到达滚动大小
	if (file_->writtenBytes() > rollSize_)
	{
		rollFile();
	}
}

bool LogFile::rollFile()
{
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  if (now > lastRoll_)
  {
    lastRoll_ = now;
    lastFlush_ = now;
    start_ = start;
    file_.reset(new DJX::FileUtil::AppendFile(filename.c_str()));
    return true;
  }
  return false;
}

string LogFile::getLogFileName(const string& basename, time_t* now)
{
  string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm); // FIXME: localtime_r ?
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;

  char tidbuf[32];
  snprintf(tidbuf, sizeof tidbuf, ".%d", DJX::CurrentThread::tid());
  filename += tidbuf;

  filename += ".log";

  return filename;
}

} // namespace DJX