#include "sqlconnpool.h"
using namespace std;

// 构造函数
SqlConnPool::SqlConnPool() {
    useCount_ = 0;
    freeCount_ = 0;
}

// 单例模式
SqlConnPool* SqlConnPool::Instance() {
    static SqlConnPool connPool;    // 创建一个静态的实例
    return &connPool;
}

// 初始化
void SqlConnPool::Init(const char* host, int port,
            const char* user,const char* pwd, const char* dbName,
            int connSize = 10) {
    assert(connSize > 0);

    // 创建 10 个 MySQL 数据库连接
    for (int i = 0; i < connSize; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql) {
            LOG_ERROR("MySql init error!");
            assert(sql);
        }

        // 连接数据库
        sql = mysql_real_connect(sql, host,
                                 user, pwd,
                                 dbName, port, nullptr, 0);
        if (!sql) {
            LOG_ERROR("MySql Connect error!");
        }
        connQue_.push(sql);
    }

    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);    // 初始化信号量
}

// 获取 MySQL数据库连接
MYSQL* SqlConnPool::GetConn() {
    MYSQL *sql = nullptr;
    if(connQue_.empty()){
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }

    // 信号量：如果 sem_wait() 函数中的值 > 0，就向下执行
    // 如果 = 0，就等待
    sem_wait(&semId_);
    {
        lock_guard<mutex> locker(mtx_);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}

// 释放一个 MySQL数据库连接，并不是真正的释放，而是把它放到连接池中
void SqlConnPool::FreeConn(MYSQL* sql) {
    assert(sql);
    lock_guard<mutex> locker(mtx_);
    connQue_.push(sql);
    sem_post(&semId_);  // 信号量 + 1
}

// 关闭数据库连接池
void SqlConnPool::ClosePool() {
    lock_guard<mutex> locker(mtx_);
    while(!connQue_.empty()) {
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    mysql_library_end();        
}

// 获取 空闲的用户数
int SqlConnPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connQue_.size();
}

// 析构函数
SqlConnPool::~SqlConnPool() {
    ClosePool();
}
