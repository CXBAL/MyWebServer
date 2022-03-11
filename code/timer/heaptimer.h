#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h> 
#include <functional> 
#include <assert.h> 
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;                 // 文件描述符
    TimeStamp expires;      // 超时时间
    TimeoutCallBack cb;     // 回调函数
    bool operator<(const TimerNode& t) {    // 重载运算符
        return expires < t.expires;
    }
};

// 小根堆是用 vector 实现的
class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }

    ~HeapTimer() { clear(); }
    
    void adjust(int id, int newExpires);     // 修改了超时时间后，需要调整节点 

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();            // 清除超时的节点

    void pop();

    int GetNextTick();

private:
    void del_(size_t i);                    // 删除元素，i 是索引
    
    void siftup_(size_t i);                 // 维护小根堆，向上调整

    bool siftdown_(size_t index, size_t n); // 向下调整

    void SwapNode_(size_t i, size_t j);     // 根据索引交换两个节点

    std::vector<TimerNode> heap_;           // 保存堆的容器

    std::unordered_map<int, size_t> ref_;   // 保存堆的元素和对应的索引
};

#endif //HEAP_TIMER_H