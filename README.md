# 介绍
Dserver是一个运行于 Linux 平台下的基于Reactor模式的多线程网络库，使用 C++11 进行编写，支持 TCP协议。
# 技术点
### 并发模型
* 使用one-loop-per-thread + thread-pool 模型，可使IO与计算任务分离。
* 使用epoll条件触发的IO多路复用技术。
* 使用socket的SO_REUSEADDR，SO_REUSEPORT选项，创建多个socket监听同一端口，并将每个socket分别加入到一个IO线程的epoll监听队列中提供服务。
* 使用eventfd + vector实现IO线程loop间通信，可以很灵活地在线程间调配负载。
### 缓冲区
* 为每个连接维护用户态InputBuffer与Ouputbuffer，以保证在非阻塞IO下的可用性。
* 使用readv系统调用结合栈上建立的临时缓冲区，提高每次读取的数据量和减少read系统调用提高性能的同时避免了每个连接的初始buffer过大造成内存浪费。
### 对象生命周期管理
* 所有内存分配操作使用智能指针，避免了内存泄露。
* 使用 RAII 的机制+智能指针将文件描述符封装成对象进行生命周期控制，避免出现文件描述符的读写操作出现串话。
### 处理定时事件
* 使用 Linux 内核提供的 timerfd 将定时事件融入IO多路复用机制和 IO 事件统一处理。
* 使用以红黑树为底层实现的std::set作为时间轮的数据结构，并在key上设法区分到期时间相同的定时器，避免使用不常见的multiset。
### 日志
* 实现基于多缓冲区技术的异步日志库。
* 使用单独的日志线程记录日志，通过多缓冲区与IO线程和计算线程通信。避免在IO线程中进行磁盘IO，使磁盘IO与计算相互重叠，降低请求处理延迟。
### 其他
* 使用gettid系统调用返回pid_t类型数据唯一标识线程，保证多个进程间的线程id唯一性，并使用__thread关键字为每个线程缓存pid_t，减小系统调用次数，提高性能。
* 在全局预留一个文件描述符，以在当打开的文件描述符超过系统限制无法caaept时可使用此文件描述符通知对端无法服务，并解决busy loop问题。

# 压测
### 运行环境
![在这里插入图片描述](https://img-blog.csdnimg.cn/a6a61cf6cc514acf87a6f43941dbbcdb.png?x-oss-process=image/watermark,type_d3F5LXplbmhlaQ,shadow_50,text_Q1NETiBASHVtYmxlSHVuZ2Vy,size_16,color_FFFFFF,t_70,g_se,x_16#pic_center)

gcc 9.4.0
Dserver开启6个IO线程
 
测试工具为 webbench1.5，测试命令如下(100个客户端持续访问15秒)。
```
./webbench -c 100 -t 15 http://127.0.0.1:8000/
```
![在这里插入图片描述](https://img-blog.csdnimg.cn/75f93e28c7834dfa822b693e33d9d3d1.png#pic_center)

