#ifndef UTILS_CGI_HPP
#define UTILS_CGI_HPP
#include "Utils_Cgi.hpp"
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>

# define BUFSIZE 10000


std::string		readFd(int fd);

#endif