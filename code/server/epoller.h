#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h> // close()
#include <vector>
#include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);  // 创建epoll对象

    ~Epoller();

    bool AddFd(int fd, uint32_t events);    // 添加文件描述符到 epoll 中进行管理

    bool ModFd(int fd, uint32_t events);    // 修改

    bool DelFd(int fd);                     // 删除

    int Wait(int timeoutMs = -1);           // 调用 epoll_wait() 进行事件检测

    int GetEventFd(size_t i) const;         // 获取产生事件的文件描述符

    uint32_t GetEvents(size_t i) const;     // 获取事件
        
private:
    int epollFd_;   // epoll_create()创建一个epoll对象，返回值就是epollFd

    std::vector<struct epoll_event> events_;     // 检测到的事件的集合 
};

#endif //EPOLLER_H