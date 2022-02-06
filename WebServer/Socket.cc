#include "Socket.h"
#include "Util.h"

#include "cstring"
#include "base/logging/Logging.h"
#include <cstdio>

void setReusePort(int fd)
{
    int opt = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void*) &opt,sizeof(opt));
}

ServerSocket::ServerSocket(int port, const char *ip) : mPort(port), mIp(ip)
{
    bzero(&mAddr, sizeof(mAddr));

    mAddr.sin_family = AF_INET;
    mAddr.sin_port = htons(port);

    if (ip != nullptr)
    {
        ::inet_pton(AF_INET, ip, &mAddr.sin_addr);
    }
    else
    {
        mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd == -1)
    {
        LOG << "creat socket error in file <" << __FILE__ << "> "
            << "at " << __LINE__ ;
        exit(0);
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
    setnonBlocking(listen_fd); 
}

void ServerSocket::bind()
{
    int ret = ::bind(listen_fd, (struct sockaddr *)&mAddr, sizeof(mAddr));
    if (ret == -1)
    {
        LOG << "bind error in file <" << __FILE__ << "> "
                  << "at " << __LINE__;
        exit(0);
    }
}

void ServerSocket::listen()
{
    int ret = ::listen(listen_fd, 1024);
    if (ret == -1)
    {
        LOG << "listen error in file <" << __FILE__ << "> "
            << "at " << __LINE__ ;
        exit(0);
    }
}

int ServerSocket::accept(ClientSocket &clientSocket) const
{
    int clientfd = ::accept(listen_fd, NULL, NULL);
    
    if (clientfd < 0)
    {
        if ((errno == EWOULDBLOCK) || (errno == EAGAIN))
            return clientfd;
        LOG << "accept error in file <" << __FILE__ << "> "
                  << "at " << __LINE__ ;
        LOG << "clientfd:" << clientfd ;
        perror("accpet error");
        exit(0);
    }

    LOG << "accept a client: " << clientfd;
    clientSocket.fd = clientfd;
    return clientfd;
}

void ServerSocket::close()
{
    if (listen_fd >= 0)
    {
        ::close(listen_fd);

        LOG << "Timer timeout closed, file descriptor:" << listen_fd;
        listen_fd = -1;
    }
}
ServerSocket::~ServerSocket()
{
    close();
}

void ClientSocket::close()
{
    if (fd >= 0)
    {
        ::close(fd);
        fd = -1;
    }
}
ClientSocket::~ClientSocket()
{
    close();
}
