#include "Utils_Cgi.hpp"



std::string readFd(int fd) {
	std::string     str;
	char            buf[BUFSIZE + 1];
	ssize_t         count;

	count = read(fd, buf, BUFSIZE);
	if (count < 0)
		throw std::runtime_error("readFd (read) error");
	buf[count] = 0;
	str.insert(str.size(), buf, count);
	return (str);
}