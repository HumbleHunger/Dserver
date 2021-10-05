#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<pthread.h>
#include<errno.h>
#include <stdlib.h>
int main(int argc,char **argv)
{
	int ret;
int serv_port;
struct sockaddr_in serv_addr;
//初始化服务器地址
memset(&serv_addr,0,sizeof(struct sockaddr_in));
serv_addr.sin_family=AF_INET;
//解析参数，获取服务器地址以及端口号
for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"-p")==0){
        serv_port=atoi(argv[i+1]);
        //检查参数合法性并记录
        if(serv_port<0 || serv_port>65535){
            printf("invalid serv_port\n");
            exit(1);
        }
        else{
            serv_addr.sin_port=htons(serv_port);
        }
        continue;
    }
    if(strcmp(argv[i],"-a")==0){
        if(inet_aton(argv[++i],&serv_addr.sin_addr)==0){
            printf("invaild serv_addr\n");
            exit(1);
        }
        continue;
    }
}
//再次检查参数是否正确
if(serv_addr.sin_port==0 || serv_addr.sin_addr.s_addr==0){
    printf("Usage: [-p] [serv_port] [-a] [serv_addr]\n");
    exit(1);
}
    printf("ip : %d , port is %d\n",serv_addr.sin_addr, serv_addr.sin_port);
//创建TCP套接字
int connfd=socket(AF_INET,SOCK_STREAM,0);
if(connfd<0){
    printf("create socket failed\n");
}
//链接服务器
if(connect(connfd,(struct sockaddr*)&serv_addr,sizeof(struct sockaddr_in))<0){
    printf("connect faild\n");
}
char wbuf[1024];
char rbuf[1024];
while(1){
    scanf("%s",wbuf);
    int n = write(connfd, wbuf, strlen(wbuf));
    printf("write n = %d\n", n);

    n = read(connfd, rbuf, strlen(rbuf));
    printf("write n = %d\n", n);

}
}
