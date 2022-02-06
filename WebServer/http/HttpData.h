#ifndef WEBSERVER_HTTP_HTTPDATA_H
#define WEBSERVER_HTTP_HTTPDATA_H

#include "../TimerNode.h"
#include "../Socket.h"

#include "HttpParse.h"
#include "HttpResponse.h"


#include <memory>


class TimerNode;

// HTTP resource
class HttpData : public std::enable_shared_from_this<HttpData> {
public:
    HttpData() : epoll_fd(-1) {

    }

public:
    std::shared_ptr<HttpRequest> request_;
    std::shared_ptr<HttpResponse> response_;
    std::shared_ptr<ClientSocket> clientSocket_;
    int epoll_fd;

    void closeTimer();

    void setTimer(std::shared_ptr<TimerNode>);

private:
    std::weak_ptr<TimerNode> timer_t;
};

#endif // WEBSERVER_HTTP_HTTPDATA_H
