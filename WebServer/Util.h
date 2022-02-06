#ifndef WEBSERVER_BASE_UTIL_H
#define WEBSERVER_BASE_UTIL_H

#pragma once
#include <string>

using namespace std;

std::string &ltrim(string &);

std::string &rtrim(string &);

std::string &trim(string &);

int setnonBlocking(int fd);
void handleForSigpipe();

int checkBasePath(char *basePath);




#endif // WEBSERVER_BASE_UTIL_H