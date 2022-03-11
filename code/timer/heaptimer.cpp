#include "heaptimer.h"

// 维护小根堆，向上调整
void HeapTimer::siftup_(size_t i) {
    assert(i >= 0 && i < heap_.size());

    size_t j = (i - 1) / 2;     // 父亲节点索引
    while(j >= 0) {
        if(heap_[j] < heap_[i]) { break; }
        SwapNode_(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

// 根据索引交换两个节点
void HeapTimer::SwapNode_(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = i;
    ref_[heap_[j].id] = j;
} 

// 向下调整
bool HeapTimer::siftdown_(size_t index, size_t n) {
    assert(index >= 0 && index < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t i = index;
    size_t j = i * 2 + 1;           // 当前节点的左子节点
    while(j < n) {
        if(j + 1 < n && heap_[j + 1] < heap_[j]) j++;
        if(heap_[i] < heap_[j]) break;
        SwapNode_(i, j);
        i = j;
        j = i * 2 + 1;
    }
    return i > index;
}

// 向堆中添加元素
// 第三个参数是一个回调函数
void HeapTimer::add(int id, int timeout, const TimeoutCallBack& cb) {
    assert(id >= 0);
    size_t i;

    if(ref_.count(id) == 0) {
        /* 新节点：堆尾插入，调整堆 */
        i = heap_.size();
        ref_[id] = i;

        // 超时时间：根据当前系统时间 和 自己设置的超时时间 来设置
        heap_.push_back({id, Clock::now() + MS(timeout), cb});
        siftup_(i); // 向上调整，跟父亲比较
    } 
    else {
        /* 已有结点：调整堆 */
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(timeout);
        heap_[i].cb = cb;
        if(!siftdown_(i, heap_.size())) {
            siftup_(i);
        }
    }
}

void HeapTimer::doWork(int id) {
    /* 删除指定id结点，并触发回调函数 */
    if(heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.cb();
    del_(i);
}

// 删除元素，参数 是索引
void HeapTimer::del_(size_t index) {
    /* 删除指定位置的结点 */
    assert(!heap_.empty() && index >= 0 && index < heap_.size());

    /* 将要删除的结点换到队尾，然后调整堆 */
    size_t i = index;
    size_t n = heap_.size() - 1;
    assert(i <= n);

    if(i < n) {
        SwapNode_(i, n);
        if(!siftdown_(i, n)) {
            siftup_(i);
        }
    }

    /* 队尾元素删除 */
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

// 修改了超时时间后，需要调整节点 
void HeapTimer::adjust(int id, int timeout) {
    /* 调整指定id的结点 */
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(timeout);;
    siftdown_(ref_[id], heap_.size());
}

// 清除超时的节点
void HeapTimer::tick() {
    /* 清除超时结点 */
    if(heap_.empty()) {
        return;
    }

    while(!heap_.empty()) {
        TimerNode node = heap_.front();

        // 如果没有超时，退出
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) { 
            break; 
        }
        node.cb();  // 如果超时了，调用回调函数，关闭连接
        pop();      // 删除超时的节点
    }
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

// 清空所有的数据
void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

// 获取下一个需要清除的节点
int HeapTimer::GetNextTick() {
    tick();             // 清除现在已有的
    size_t res = -1;
    
    if(!heap_.empty()) {
        res = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
        if(res < 0) { res = 0; }
    }
    return res;
}