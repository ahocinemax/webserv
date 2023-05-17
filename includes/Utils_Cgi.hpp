#ifndef UTILS_CGI_HPP
#define UTILS_CGI_HPP
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include <signal.h>

# define BUFSIZE 10000

extern int child;
std::string		readFd(int fd);
bool	AccessiblePath(const std::string& path);
void	SigpipeIgnore(int sig);
void	SigpipeSet(int state);

#endif