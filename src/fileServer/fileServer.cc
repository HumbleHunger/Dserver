#include "../net/EventLoop.h"
#include "../net/TcpServer.h"
#include "../net/TcpConnection.h"
#include <fstream>
#include <cstring>
#include <map>
#include <string>
#include <iostream>

using namespace DJX;
using namespace DJX::net;

std::map<std::string, std::string> passwordMap;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("Client connected\n");
    }
    else
    {
        printf("Client disconnected\n");
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
    while (buf->readableBytes() > 0)
    {
        // 判断当前缓冲区中是否有一个完整的请求
        if (buf->readableBytes() < 4) return;

        // 读取请求头中的数据包长度
        int32_t len = *reinterpret_cast<const int32_t*>(buf->peek());

        // 判断当前缓冲区中是否有一个完整的请求
        if (buf->readableBytes() < len + 4) return;

        // 读取数据包中的数据
        string message(buf->peek() + 4, len);
        printf("Server received %zd bytes at %s\n", message.size(), receiveTime.toFormattedString().c_str());
        std::cout << "Server recv data " << message << std::endl;
        // 处理客户端上传请求
        if (message.substr(0, 6) == "UPLOAD")
        {
            string nameLen = message.substr(6, 4);
            int* lenp = (int*)nameLen.c_str();
            std::cout << "filename len is " << *lenp << std::endl;
            string filename = message.substr(10, *lenp);
            string password = message.substr(10 + *lenp, len - 10 - *lenp);
            passwordMap[filename] = password;
        }

        // 处理客户端下载请求
        if (message.substr(0, 8) == "DOWNLOAD")
        {
            string nameLen = message.substr(8, 4);
            int* lenp = (int*)nameLen.c_str();
            std::cout << "filename len is " << *lenp << std::endl;
            string filename = message.substr(12, *lenp);
            string password = message.substr(12 + *lenp, len - 12 - *lenp);
            std::cout << "filename is " << filename << std::endl;
            std::cout << "password is " << password << std::endl;

            if (passwordMap.count(filename) == 0 || passwordMap[filename] != password) {
                if (passwordMap.count(filename) == 0) {
                    std::cout << "file not exist" << std::endl;
                } else {
                    std::cout << "password error" << std::endl;
                }
                string data = "0";
                conn->send(data.c_str(), 1);
            } else {
                printf("Server will download file: %s\n", filename.c_str());
                int count = 0;
                FILE *fp = fopen(filename.c_str(), "rb");
                unsigned char buffer[1024];
                memset(&buffer,0,sizeof(buffer));
                while((len=fread(buffer, sizeof(unsigned char), 512, fp))>0)
                {
                    string data = "1";
                    char lenp[4];
                    memset(lenp, 0, 4);
                    sprintf(lenp, "%d", len);
                    for (int i = 0; i < 4; ++i) {
                        data.push_back(lenp[i]);
                    }
                    
                    for (int i = 0; i < len; ++i) {
                        data.push_back(buffer[i]);
                        std::cout << buffer[i];
                    }

                    conn->send(data.c_str(), data.size());
                    count += len;
                    memset(&buffer,0,sizeof(buffer));
                }
                /*
                std::ifstream fin(filename, std::ios::binary | std::ios::in);

                if (!fin)
                {
                    printf("Open file %s error!\n", filename.c_str());
                    return;
                }

                char buffer[1024];
                memset(buffer, 0, sizeof(buffer));
                int count = 0;

                while (fin.read(buffer, sizeof(buffer)).gcount() > 0)
                {
                    count += fin.gcount();
                    string data = "1";

                    char len[4];
                    memset(len, 0, 4);
                    sprintf(len, "%ld", fin.gcount());
                    for (int i = 0; i < 4; ++i) {
                        data.push_back(len[i]);
                    }

                    //std::cout << buffer << std::endl;

                    data += string(buffer);
                    std::cout << "send len " << data.size() << std::endl;
                    std::cout << "send data " << data << std::endl;
                    conn->send(data.c_str(), data.size());
                }
*/
                string data = "0";
                conn->send(data.c_str(), 1);
                data = "0";
                conn->send(data.c_str(), 1);
                data = "0";
                conn->send(data.c_str(), 1);

                fclose(fp);
                printf("Server download file %s finished, size: %d bytes\n", filename.c_str(), count);
            }
        }

        // 处理客户端上传请求
        if (message.substr(0, 4) == "FILE")
        {
            string nameLen = message.substr(4, 4);
            int* lenp = (int*)nameLen.c_str();
            std::cout << "filename len is " << *lenp << std::endl;
            string filename = message.substr(8, *lenp);
            printf("Server will upload file: %s\n", filename.c_str());
            std::ofstream fout(filename, std::ios::binary | std::ios::out);

            if (!fout)
            {
                printf("Open file %s error!\n", filename.c_str());
                return;
            }

            fout.write(message.data() + 8 + *lenp, len - 8 - *lenp);
            fout.close();
            printf("Server upload file %s finished\n", filename.c_str());
        
            // 回复客户端上传结果
            conn->send(string("Upload file finished"));
        }

        // 更新缓冲区
        buf->retrieve(len + 4);
    }
}

int main(int argc, char* argv[])
{
    EventLoop loop;
    TcpServer server(&loop, InetAddress(8001));

    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setThreadNum(6);
    server.start();
    loop.loop();
}