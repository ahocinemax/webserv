#ifndef UTILS_CGI_HPP
#define UTILS_CGI_HPP
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <unistd.h>
#include <signal.h>
#include <stdexcept>

extern int child;
bool	AccessiblePath(const std::string& path);
void	SigpipeIgnore(int sig);
void	SigpipeSet(int state);

#endif