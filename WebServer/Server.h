#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include "Socket.h"

#include "http/HttpParse.h"
#include "http/HttpResponse.h"
#include "http/HttpData.h"
#include "base/Noncopyable.h"

#define BUFFER_SIZE 4096

class HttpServer : public noncopyable
{
public:
    enum FileStatus
    {
        FILE_OK,
        FILE_NOT_FOUND,
        FILE_FORBIDDEN,
    };

    explicit HttpServer(int port = 80,const char* ip = nullptr)
        :  serverSocket(port,ip)
    {
        serverSocket.bind();
        serverSocket.listen();
    }

    void run(int thread_num, int max_queue_size = 100000);

    void doRequest(std::shared_ptr<void> arg);  

private:
    ServerSocket serverSocket;

    void header(std::shared_ptr<HttpData> httpData);

    FileStatus statusFile(std::shared_ptr<HttpData> httpData, const char* basepath);

    void send(std::shared_ptr<HttpData> httpData,FileStatus fileStatus);

    void getHttpData(std::shared_ptr<HttpData> httpData);

    void handleIndex();
};




































#endif // WEBSERVER_SERVER_H