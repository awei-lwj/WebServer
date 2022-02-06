cmake_minimum_required(VERSION 3.5)
project(webserver)

set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS
        -Wall
        -std=c++11
        -lpthread
        -Wno-unused-parameter
        -O3
        )

link_libraries(pthread)

set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O3")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


set(SOURCE_FILES
        WebServer/base/logging/AsyncLogging.cc
        WebServer/base/logging/FileUtil.cc
        WebServer/base/logging/LogFile.cc
        WebServer/base/logging/LogStream.cc
        WebServer/base/logging/Logging.cc

        WebServer/base/thread/Thread.cc
        WebServer/base/thread/ThreadPool.cc

        WebServer/http/HttpRequest.cc
        WebServer/http/HttpData.cc
        WebServer/http/HttpParse.cc
        WebServer/http/HttpResponse.cc

        WebServer/Epoll.cc
        WebServer/EventThreadPool.cc
        WebServer/Server.cc
        WebServer/Socket.cc
        WebServer/TimerNode.cc
        WebServer/TimerQueue.cc
        WebServer/Util.cc


        WebServer/main.cc
        )

add_executable(webserver  ${SOURCE_FILES})