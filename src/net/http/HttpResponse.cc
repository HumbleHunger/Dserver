#include "HttpResponse.h"
#include "../Buffer.h"

#include <stdio.h>

using namespace DJX;
using namespace DJX::net;

void HttpResponse::appendToBuffer(Buffer* output) const
{
  char buf[32];
  snprintf(buf, sizeof buf, "HTTP/1.1 %d ", statusCode_);
  output->append(buf);
  output->append(statusMessage_.c_str());
  output->append("\r\n");

  if (closeConnection_)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", body_.size());
    output->append(buf);
    output->append("Connection: Keep-Alive\r\n");
  }

  for (const auto& header : headers_)
  {
    output->append(header.first.c_str());
    output->append(": ");
    output->append(header.second.c_str());
    output->append("\r\n");
  }

  output->append("\r\n");
  output->append(body_.c_str());
}