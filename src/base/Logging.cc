#include "Logging.h"

#include "CurrentThread.h"
#include "Timestamp.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace DJX
{
// 定义全局的默认日志等级
Logger::LogLevel g_logLevel = Logger::INFO;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

class T
{
 public:
  T(const char* str, unsigned len)
    :str_(str),
     len_(len)
  {
    assert(strlen(str) == len_);
  }

  const char* str_;
  const unsigned len_;
};

} // namespace DJX