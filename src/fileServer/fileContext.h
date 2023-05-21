#ifndef DJX_NET_file_fileCONTEXT_H
#define DJX_NET_file_fileCONTEXT_H

#include "../base/copyable.h"

#include "fileRequest.h"

namespace DJX
{
namespace net
{

class Buffer;

class fileContext : public DJX::copyable
{
 public:
  enum fileRequestParseState
  {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  fileContext()
    : state_(kExpectRequestLine)
  {
  }

  // default copy-ctor, dtor and assignment are fine

  // return false if any error
  bool parseRequest(Buffer* buf, Timestamp receiveTime);

  bool gotAll() const
  { return state_ == kGotAll; }

  void reset()
  {
    state_ = kExpectRequestLine;
    fileRequest dummy;
    request_.swap(dummy);
  }

  const fileRequest& request() const
  { return request_; }

  fileRequest& request()
  { return request_; }

 private:
  bool processRequestLine(const char* begin, const char* end);

  fileRequestParseState state_;
  fileRequest request_;
};

}  // namespace net
}  // namespace DJX

#endif 