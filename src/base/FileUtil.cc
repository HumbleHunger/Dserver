#include "FileUtil.h"
#include "Logging.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

namespace DJX
{
namespace FileUtil
{

AppendFile::AppendFile(std::string filename)
	: fp_(::fopen(filename.c_str(), "ae")),
	  writtenBytes_(0)
{
	assert(fp_);
	// 设置文件流的缓冲区
	::setbuffer(fp_, buffer_, sizeof buffer_);
}

AppendFile::~AppendFile()
{
	::fclose(fp_);
}

void AppendFile::append(const char* logline, const size_t len)
{
  	size_t n = write(logline, len);
  	size_t remain = len - n;
	// 循环写入一直到写完
  	while (remain > 0)
  	{
  	  	size_t x = write(logline + n, remain);
  	  	if (x == 0)
  	  	{
  	  		int err = ferror(fp_);
  	    	if (err)
  	    	{
  	    	  fprintf(stderr, "AppendFile::append() failed %s\n", DJX::strerror_tl(err));
  	    	}
  	    break;
	}
    n += x;
    remain = len - n; // remain -= x
  	}

  	writtenBytes_ += len;
}
// 将文件流缓冲区的东西刷新
void AppendFile::flush()
{
	::fflush(fp_);
}

size_t AppendFile::write(const char* logline, size_t len)
{
	// 线程不安全的fwrite
	return ::fwrite_unlocked(logline, 1, len, fp_);
}

} // namespace FileUtil
} // namespace DJX
