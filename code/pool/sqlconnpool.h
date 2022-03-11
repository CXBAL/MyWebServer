#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

// 数据库连接池
class SqlConnPool {
public:
    static SqlConnPool *Instance();

    MYSQL *GetConn();               // 获取 MySQL数据库连接
    void FreeConn(MYSQL * conn);    // 释放一个 MySQL数据库连接，并不是真正的释放，而是把它放到连接池中
    int GetFreeConnCount();         // 获取 空闲的用户数

    void Init(const char* host, int port,
              const char* user,const char* pwd, 
              const char* dbName, int connSize);    // 初始化
    void ClosePool();               // 关闭数据库连接池

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;  // 最大的连接数
    int useCount_;  // 当前的用户数
    int freeCount_; // 空闲的用户数

    std::queue<MYSQL *> connQue_;   // 队列（MySQL *），用来操作数据库
    std::mutex mtx_;    // 互斥锁
    sem_t semId_;   // 信号量
};


#endif // SQLCONNPOOL_H