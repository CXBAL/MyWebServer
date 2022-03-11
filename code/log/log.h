#ifndef LOG_H
#define LOG_H

#include <mutex>
#include <string>
#include <thread>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         //mkdir
#include "blockqueue.h"
#include "../buffer/buffer.h"

class Log {
public:

    // maxQueueCapacity 是 设置阻塞队列的大小
    void init(int level, const char* path = "./log", 
                const char* suffix =".log",
                int maxQueueCapacity = 1024);

    static Log* Instance();
    static void FlushLogThread();

    void write(int level, const char *format,...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen() { return isOpen_; }
    
private:
    Log();
    void AppendLogLevelTitle_(int level);
    virtual ~Log();
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256;    // 日志路径长度
    static const int LOG_NAME_LEN = 256;    // 日志文件名长度
    static const int MAX_LINES = 50000;     // 日志最多能保存 50000 行数据，如果超过了，重新开一个文件

    const char* path_;      // 路径   
    const char* suffix_;    // 文件名的后缀

    int MAX_LINES_;     // 就是 之前定义的 50000

    int lineCount_;     // 当前日志已经写了多少行数据
    int toDay_;         // 记录今天的日期

    bool isOpen_;       // 日志是否打开
 
    Buffer buff_;       // 写日志也是往 Buffer 中去写
    int level_;         // 日志级别
    bool isAsync_;      // 是否异步

    FILE* fp_;                                          // 文件指针，去操作文件
    std::unique_ptr<BlockDeque<std::string>> deque_;    // 封装的 阻塞队列
    std::unique_ptr<std::thread> writeThread_;          // 开启子线程去写日志
    std::mutex mtx_;                                    // 互斥量
};

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);

#endif //LOG_H