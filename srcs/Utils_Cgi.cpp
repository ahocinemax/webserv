#include "Utils_Cgi.hpp"


std::string readFd(int fd) {
	std::string     str;
	char            buf[BUFSIZE + 1];
	ssize_t         count;

	count = read(fd, buf, BUFSIZE);
	if (count < 0)
		throw std::runtime_error("readFd: (read) error");
	buf[count] = 0;
	str.insert(str.size(), buf, count);
	return (str);
}

void	SigPipeIgnore(int sig) {
	(void) sig;
}

void	SigpipeSet(int state) {
	void	(*oldHandler_SigPipe)(int) = 0;

	if (state == 0) {
		oldHandler_SigPipe = signal(SIGPIPE, SigPipeIgnore);
	} else if (state == 1) {
		signal(SIGPIPE, oldHandler_SigPipe);
	}
}

bool	AccessiblePath(const std::string& path)
{
	int	ret;

	ret = access(path.c_str(), F_OK); 
	return (!ret);
}