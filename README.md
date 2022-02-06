# WebServer
A C++ Lightweight Web Server based on Linux epoll 
# A C++ Lightweight Web Server

## 简介
模仿muduo库实现的一个轻量级http WebServer，是一个C++11并使用了Boost扩展库编写的Web服务器，解析了get、head请求，可以处理静态资源，支持HTTP请求，并实现了异步日志，记录了服务器运行状态。

## Key
Key： Linux C、Linux 多线程网络编程、UNIX网络编程，C/C++ 11

## 开发部署的环境
 - 操作系统： Ubuntu 20.04
 - 编译器： gcc
 - 版本控制： git
 - 自动化给构建： cmake
 - 集成开发工具： vscode
 - 编辑器：Vim
 - 压测工具： WebBench

## Usage

```bash
./build.sh
./webserver [-p port] [-t thread_numbers]  [-r website_root_path] [-d daemon_run]
```

## 核心技术

 1. 状态机解析HTTP请求，支持 HTTP GET、HEAD，支持HTTP长连接，定时回调handler处理超时连接
 2. 使用 priority queue 实现的最小堆结构管理定时器，使用标记删除，以支持惰性删除，提高性能
 3. 并发模型：epoll + 非阻塞IO + 边缘触发(ET) ，使用Reactor编程模型，用one peer through Loop的方式保证一个线程处理一个请求
 4. 使用线程池提高并发度，并降低频繁创建线程的开销
 5. 使用C++ 11中的只能指针中的shared_ptr、weak_ptr管理资源对象，防止内存泄漏
 6. 使用RAII手法封装互斥器(pthread_mutex_t)、 条件变量(pthread_cond_t)等线程同步互斥机制，使用RAII管理文件描述符等资源，并同时使用类mutexLockGuard避免锁争用
 7. 使用双缓冲区技术实现了简单的异步日志系统

## TODO
集群系统

