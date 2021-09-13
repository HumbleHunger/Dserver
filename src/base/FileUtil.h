#ifndef DJX_FILEUTIL_H
#define DJX_FILEUTIL_H

#include "noncopyable.h"
#include <stdio.h>
#include <sys/types.h>  // for off_t
#include <string>

namespace DJX
{
namespace FileUtil
{
// 追加文件操作的封装类
class AppendFile : noncopyable
{
public:
	explicit AppendFile(std::string filename);

	~AppendFile();

	void append(const char* logline, size_t len);

	void flush();

	off_t writtenBytes() const { return writtenBytes_; }

private:
	
	size_t write(const char* logline, size_t len);

	FILE* fp_;
	// 64kb
	char buffer_[64*1024];
	// 已写入的字节数
	off_t writtenBytes_;

};

} // namespace FileUtil

} // namespace DJX

#endif