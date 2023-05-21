#include <../net/TcpServer.h>
#include <../net/EventLoop.h>
#include <../net/Endian.h>
#include <../base/Logging.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

using namespace DJX;
using namespace DJX::net;
using namespace std;
using namespace std::placeholders;

const int kBufferSize = 65536; // 缓冲区大小

class FileServer
{
public:
    FileServer(EventLoop* loop, const InetAddress& listenAddr)
        : server_(loop, listenAddr)
    {
        server_.setConnectionCallback(
            std::bind(&FileServer::onConnection, this, _1));
        server_.setMessageCallback(
            std::bind(&FileServer::onMessage, this, _1, _2, _3));
    }

    void start()
    {
        server_.start();
    }

private:
    void onConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << "FileServer - " << conn->peerAddress().toIpPort() 
            << " -> " << conn->localAddress().toIpPort() 
            << " is " << (conn->connected() ? "UP" : "DOWN");
    }

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
    {
        while (buf->readableBytes() >= sizeof(int32_t))
        {
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data);
            const int32_t len = DJX::net::socketOps::networkToHost32(be32);
            if (buf->readableBytes() >= len + sizeof(int32_t))
            {
                buf->retrieve(sizeof(int32_t));
                const void* filename = buf->peek();
                std::string fname(static_cast<const char*>(filename), len);
                buf->retrieve(len);
                LOG_INFO << "FileServer - Receive file: " << fname;
                int fd = ::open(fname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0)
                {
                    LOG_ERROR << "FileServer - Cannot open file " << fname;
                    conn->shutdown();
                    return;
                }
                while (buf->readableBytes() > 0)
                {
                    int n = ::write(fd, buf->peek(), buf->readableBytes());
                    if (n > 0)
                    {
                        buf->retrieve(n);
                    }
                    else
                    {
                        LOG_ERROR << "FileServer - Write file " << fname << " error";
                        ::close(fd);
                        conn->shutdown();
                        return;
                    }
                }
                ::close(fd);
                LOG_INFO << "FileServer - Complete file: " << fname;
                conn->send("OK\n");
            }
            else
            {
                break;
            }
        }
    }

    TcpServer server_;
};

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    }
    else
    {
        LOG_INFO << "FileServer - pid = " << getpid() << ", tid = " << CurrentThread::tid();
        EventLoop loop;
        FileServer server(&loop, InetAddress(atoi(argv[1])));
        server.start();
        loop.loop();
    }
    return 0;
}