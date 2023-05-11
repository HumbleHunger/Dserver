#include "fileServer.h"

#include "../base/Logging.h"
#include "fileContext.h"
#include "fileRequest.h"
#include "fileResponse.h"

using namespace DJX;
using namespace DJX::net;
using namespace std::placeholders;

namespace DJX
{
namespace net
{
namespace detail
{

void defaultfileCallback(const fileRequest&, fileResponse* resp)
{
  resp->setStatusCode(fileResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}  // namespace detail
}  // namespace net
}  // namespace DJX

fileServer::fileServer(EventLoop* loop,
                       const InetAddress& listenAddr)
  : server_(loop, listenAddr),
    fileCallback_(detail::defaultfileCallback)
{
  server_.setConnectionCallback(
      std::bind(&fileServer::onConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&fileServer::onMessage, this, _1, _2, _3));
}

void fileServer::start()
{
  LOG_WARN << "fileServer[" << server_.getLoop()
    << "] starts listening on " << server_.ipPort();
  server_.start();
}

void fileServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setContext(fileContext());
  }
}

void fileServer::onMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receiveTime)
{
  fileContext* context = boost::any_cast<fileContext>(conn->getMutableContext());

  if (!context->parseRequest(buf, receiveTime))
  {
    conn->send("file Bad Request\r\n\r\n");
    conn->shutdown();
  }

  // 获取到完整的一个request
  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void fileServer::onRequest(const TcpConnectionPtr& conn, const fileRequest& req)
{
  const string& connection = req.getHeader("Connection");
  bool close = connection == "close" ||
    (req.getVersion() == fileRequest::kfile10 && connection != "Keep-Alive");
  fileResponse response(close);
  fileCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(buf);
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}