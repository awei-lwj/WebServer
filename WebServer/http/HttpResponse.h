// #ifndef WEBSERVER_HTTP_HTTPRESPONSE_H
// #define WEBSERVER_HTTP_HTTPRESPONSE_H

#pragma once
#include "HttpRequest.h"

#include <string>
#include <unordered_map>
#include <memory>

struct HttpType
{
    HttpType(const std::string &str) : type(str){};
    HttpType(const char *str) : type(str){};

    std::string type;
};

extern std::unordered_map<std::string, HttpType> Mime_map;

class HttpResponse
{
public:
    enum HttpStatusCode
    {
        UnKnown,
        k200_OK = 200,
        k403_ForBiden = 403,
        k404_NotFound = 404
    };

    explicit HttpResponse(bool keep = true)
        : statusCode_t(UnKnown),
          keepAlive(keep),
          type_t("text/html"),
          body(nullptr),
          version_t(HttpRequest::HTTP_11)
    {
    }

    ~HttpResponse()
    {
        if (body != nullptr)
            delete[] body;
    }

    void setStatusCode(HttpStatusCode code)
    {
        statusCode_t = code;
    }

    void setBody(const char *buf)
    {
        body = buf;
    }

    void setContentLength(int len)
    {
        contentLength = len;
    }

    void setVersion(const HttpRequest::HTTP_VERSION &version)
    {
        version_t = version;
    }

    void setStatusMsg(const std::string &msg)
    {
        statusMessage = msg;
    }

    void setFilePath(const std::string &path)
    {
        filePath_t = path;
    }

    void setMime(const HttpType &mime)
    {
        type_t = mime;
    }

    void setKeepAlive(bool isalive)
    {
        keepAlive = isalive;
    }

    void addHeader(const std::string &key, const std::string &value)
    {
        headers[key] = value;
    }

    bool keep_alive() const
    {
        return keepAlive;
    }

    const HttpRequest::HTTP_VERSION version() const
    {
        return version_t;
    }

    const std::string &filePath() const
    {
        return filePath_t;
    }

    HttpStatusCode statusCode() const
    {
        return statusCode_t;
    }

    const std::string &statusMsg() const
    {
        return statusMessage;
    }

    void appendBuffer(char *) const;

private:
    HttpStatusCode statusCode_t;
    HttpRequest::HTTP_VERSION version_t;
    std::string statusMessage;

    bool keepAlive;
    HttpType type_t;
    const char *body;
    int contentLength;

    std::string filePath_t;
    std::unordered_map<std::string, std::string> headers;
};

// #endif // WEBSERVER_HTTP_HTTPRESPONSE_H