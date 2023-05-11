#ifndef DJX_file_fileSERVER_H
#define DJX_file_fileSERVER_H

#include "../net/TcpServer.h"

namespace DJX
{
namespace net
{

class fileRequest;
class fileResponse;

/// A simple embeddable file server designed for report status of a program.
/// It is not a fully file 1.1 compliant server, but provides minimum features
/// that can communicate with fileClient and Web browser.
/// It is synchronous, just like Java Servlet.
class fileServer : noncopyable
{
 public:
  typedef std::function<void (const fileRequest&,
                              fileResponse*)> fileCallback;

  fileServer(EventLoop* loop,
             const InetAddress& listenAddr);

  EventLoop* getLoop() const { return server_.getLoop(); }

  void setfileCallback(const fileCallback& cb)
  {
    fileCallback_ = cb;
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const fileRequest&);

  TcpServer server_;
  fileCallback fileCallback_;
};

}  // namespace net
}  // namespace DJX

#endif 