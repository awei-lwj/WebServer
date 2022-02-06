#include "EventThreadPool.h"
#include "Epoll.h"
#include "Util.h"
#include "Server.h"

#include "http/HttpParse.h"
#include "http/HttpResponse.h"
#include "http/HttpData.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <string>
#include <functional>
#include <sys/epoll.h>
#include <vector>
#include <cstring>

char NOT_FOUND_PAGE[] =
    "<html>\n"
    "<head><title>404 Not Found</title></head>\n"
    "<body bgcolor=\"white\">\n"
    "<center><h1>404 Not Found</h1></center>\n"
    "<hr><center>LC WebServer/0.3 (Linux)</center>\n"
    "</body>\n"
    "</html>";

char FORBIDDEN_PAGE[] =
    "<html>\n"
    "<head><title>403 Forbidden</title></head>\n"
    "<body bgcolor=\"white\">\n"
    "<center><h1>403 Forbidden</h1></center>\n"
    "<hr><center>LC WebServer/0.3 (Linux)</center>\n"
    "</body>\n"
    "</html>";

char INDEX_PAGE[] =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <title>Welcome to LC WebServer!</title>\n"
    "    <style>\n"
    "        body {\n"
    "            width: 35em;\n"
    "            margin: 0 auto;\n"
    "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
    "        }\n"
    "    </style>\n"
    "</head>\n"
    "<body>\n"
    "<h1>Welcome to LC WebServer!</h1>\n"
    "<p>If you see this page, the lc webserver is successfully installed and\n"
    "    working. </p>\n"
    "\n"
    "<p>For online documentation and support please refer to\n"
    "    <a href=\"https://github.com/MarvinLe/WebServer\">LC WebServer</a>.<br/>\n"
    "\n"
    "<p><em>Thank you for using LC WebServer.</em></p>\n"
    "</body>\n"
    "</html>";

char TEST[] = "HELLO AWEI";

std::unordered_map<std::string, HttpType> Mime_map;

extern std::string basePath;

void HttpServer::run(int thread_num, int max_queque_size)
{
    EventThreadPool threadPool(thread_num, max_queque_size);

    //        ClientSocket *clientSocket = new ClientSocket;
    //        serverSocket.accept(*clientSocket);
    //        thread::ThreadTask *threadTask = new ThreadTask;
    //        threadTask->process = std::bind(&HttpServer::do_request, this, std::placeholders::_1);
    //        threadTask->arg = static_cast<void*>(clientSocket);
    //        threadPool.append(threadTask);

    int epoll_fd = Epoll::init(1024);
    LOG << "a|epoll_fd=" << epoll_fd;
    int ret = setnonBlocking(epoll_fd);

    if (ret < 0)
    {
        LOG << "epoll_fd set nonblocking error";
    }

    std::shared_ptr<HttpData> httpData(new HttpData());
    httpData->epoll_fd = epoll_fd;
    serverSocket.epoll_fd = epoll_fd;

    __uint32_t event = (EPOLLIN | EPOLLET);
    Epoll::epollAdd(epoll_fd, serverSocket.listen_fd, event, httpData);

    while (true)
    {

        //        epoll_event eventss;
        //        epoll_event events[1024];
        //        eventss.data.fd = serverSocket.listen_fd;
        //        eventss.events = EPOLLIN | EPOLLET;
        //
        //        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket.listen_fd, &eventss);
        //        int ret = epoll_wait(epoll_fd, events, 1024, -1);
        //        if (ret > 0) {
        //            std::cout << "ret =" << ret << std::endl;
        //        } else {
        //            std::cout << "ret =" << ret << std::endl;
        //        }

        // test end

        std::vector<std::shared_ptr<HttpData>> events = Epoll::poll(serverSocket, 1024, -1);

        for (auto &req : events)
        {
            threadPool.append(req, std::bind(&HttpServer::doRequest, this, std::placeholders::_1));
        }
        // 处理定时器超时事件
        Epoll::timerQueue.handleExpiredEvents();
    }
}

void HttpServer::doRequest(std::shared_ptr<void> arg)
{
    std::shared_ptr<HttpData> sharedHttpData = std::static_pointer_cast<HttpData>(arg);

    char buffer[BUFFER_SIZE];

    bzero(buffer, BUFFER_SIZE);
    int check_index = 0, read_index = 0, start_line = 0;
    ssize_t recv_data;
    HttpRequestParser::PARSE_STATE parse_state = HttpRequestParser::PARSE_REQUESTLINE;

    while (true)
    {

        recv_data = recv(sharedHttpData->clientSocket_->fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
        if (recv_data == -1)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                return;
            }

            LOG << "reading failed";
            return;
        }

        if (recv_data == 0)
        {
            LOG << "connection closed by peer";
            break;
        }
        read_index += recv_data;

        HttpRequestParser::HTTP_CODE retcode = HttpRequestParser::parse_content(
            buffer, check_index, read_index, parse_state, start_line, *sharedHttpData->request_);

        if (retcode == HttpRequestParser::NO_REQUEST)
        {
            continue;
        }

        if (retcode == HttpRequestParser::GET_REQUEST)
        {
            auto it = sharedHttpData->request_->mHeaders.find(HttpRequest::Connection);
            if (it != sharedHttpData->request_->mHeaders.end())
            {
                if (it->second == "keep-alive")
                {
                    sharedHttpData->response_->setKeepAlive(true);
                    sharedHttpData->response_->addHeader("Keep-Alive", std::string("timeout=20"));
                }
                else
                {
                    sharedHttpData->response_->setKeepAlive(false);
                }
            }
            header(sharedHttpData);
            getHttpData(sharedHttpData);

            FileStatus fileStatus = statusFile(sharedHttpData, basePath.c_str());
            send(sharedHttpData, fileStatus);
            // 如果是keep_alive else sharedHttpData将会自动析构释放clientSocket，从而关闭资源
            if (sharedHttpData->response_->keep_alive())
            {
                LOG << "再次添加定时器  keep_alive: " << sharedHttpData->clientSocket_->fd;
                Epoll::epollModify(sharedHttpData->epoll_fd, sharedHttpData->clientSocket_->fd, Epoll::DEFAULT_EVENTS, sharedHttpData);
                Epoll::timerQueue.addTimer(sharedHttpData, TimerQueue::DEFAULT_TIMEOUT);
            }
        }
        else
        {
            LOG << "Bad Request";
        }
    }
}

void HttpServer::header(std::shared_ptr<HttpData> httpData)
{
    if (httpData->request_->mVersion == HttpRequest::HTTP_11)
    {
        httpData->response_->setVersion(HttpRequest::HTTP_11);
    }
    else
    {
        httpData->response_->setVersion(HttpRequest::HTTP_10);
    }
    httpData->response_->addHeader("Server", "LC WebServer");
}

// 获取HttpData 同时设置path到response
void HttpServer::getHttpData(std::shared_ptr<HttpData> httpData)
{
    std::string filepath = httpData->request_->mUri;
    std::string mime;
    int pos;

    LOG << "uri: " << filepath;
    if ((pos = filepath.rfind('?')) != std::string::npos)
    {
        filepath.erase(filepath.rfind('?'));
    }

    if (filepath.rfind('.') != std::string::npos)
    {
        mime = filepath.substr(filepath.rfind('.'));
    }
    //FIXME:
    decltype(Mime_map)::iterator it;

    if ((it = Mime_map.find(mime)) != Mime_map.end())
    {
        httpData->response_->setMime(it->second);
    }
    else
    {
        httpData->response_->setMime(Mime_map.find("default")->second);
    }

    httpData->response_->setFilePath(filepath);
}

HttpServer::FileStatus HttpServer::statusFile(std::shared_ptr<HttpData> httpData, const char *basepath)
{
    struct stat file_stat;
    char file[strlen(basepath) + strlen(httpData->response_->filePath().c_str()) + 1];
    strcpy(file, basepath);
    strcat(file, httpData->response_->filePath().c_str());

    // 文件不存在
    if (httpData->response_->filePath() == "/" || stat(file, &file_stat) < 0)
    {
        httpData->response_->setMime(HttpType("text/html"));
        if (httpData->response_->filePath() == "/")
        {
            httpData->response_->setStatusCode(HttpResponse::k200_OK);
            httpData->response_->setStatusMsg("OK");
        }
        else
        {
            httpData->response_->setStatusCode(HttpResponse::k404_NotFound);
            httpData->response_->setStatusMsg("Not Found");
        }
        return FILE_NOT_FOUND;
    }

    // 不是普通文件或无访问权限
    if (!S_ISREG(file_stat.st_mode))
    {

        httpData->response_->setMime(HttpType("text/html"));
        httpData->response_->setStatusCode(HttpResponse::k403_ForBiden);
        httpData->response_->setStatusMsg("ForBidden");
    
        LOG << "not normal file";
        return FILE_FORBIDDEN;
    }

    httpData->response_->setStatusCode(HttpResponse::k200_OK);
    httpData->response_->setStatusMsg("OK");
    httpData->response_->setFilePath(file);
    //    std::cout << "文件存在 - ok" << std::endl;
    return FILE_OK;
}

void HttpServer::send(std::shared_ptr<HttpData> httpData, FileStatus fileState)
{
    char header[BUFFER_SIZE];
    bzero(header, '\0');
    const char *internal_error = "Internal Error";
    struct stat file_stat;
    httpData->response_->appendBuffer(header);

    // 404
    if (fileState == FILE_NOT_FOUND)
    {

        // 如果是 '/'开头就发送默认页
        if (httpData->response_->filePath() == std::string("/"))
        {
            // 现在使用测试页面
            sprintf(header, "%sContent-length: %d\r\n\r\n", header, strlen(INDEX_PAGE));
            sprintf(header, "%s%s", header, INDEX_PAGE);
        }
        else
        {
            sprintf(header, "%sContent-length: %d\r\n\r\n", header, strlen(NOT_FOUND_PAGE));
            sprintf(header, "%s%s", header, NOT_FOUND_PAGE);
        }
        ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
        return;
    }

    if (fileState == FILE_FORBIDDEN)
    {
        sprintf(header, "%sContent-length: %d\r\n\r\n", header, strlen(FORBIDDEN_PAGE));
        sprintf(header, "%s%s", header, FORBIDDEN_PAGE);
        ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
        return;
    }
    // 获取文件状态
    if (stat(httpData->response_->filePath().c_str(), &file_stat) < 0)
    {
        sprintf(header, "%sContent-length: %d\r\n\r\n", header, strlen(internal_error));
        sprintf(header, "%s%s", header, internal_error);
        ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
        return;
    }

    int filefd = ::open(httpData->response_->filePath().c_str(), O_RDONLY);
    // 内部错误
    if (filefd < 0)
    {
        std::cout << "打开文件失败" << std::endl;
        sprintf(header, "%sContent-length: %d\r\n\r\n", header, strlen(internal_error));
        sprintf(header, "%s%s", header, internal_error);
        ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
        close(filefd);
        return;
    }

    sprintf(header, "%sContent-length: %d\r\n\r\n", header, file_stat.st_size);
    ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
    void *mapbuf = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0);
    ::send(httpData->clientSocket_->fd, mapbuf, file_stat.st_size, 0);
    munmap(mapbuf, file_stat.st_size);
    close(filefd);
    return;
    
err:
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, strlen(internal_error));
    sprintf(header, "%s%s", header, internal_error);
    ::send(httpData->clientSocket_->fd, header, strlen(header), 0);
    return;
}
