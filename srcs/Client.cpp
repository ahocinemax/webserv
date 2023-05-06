#include "Client.hpp"

Client::Client(Server *server)
{
	_server = server;
	_addrLen = sizeof(_addr);
	clearRequest();
	gettimeofday(&_timer, NULL);
	memset(_request, 0, MAX_REQUEST_SIZE + 1);
}

Client::~Client(void) {}

void	Client::setTimer(struct timeval &timer) { _timer = timer; }

void	Client::setSocket(int socket)
{
	_socket = socket;
	fcntl(_socket, F_SETFL, O_NONBLOCK);
	if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&_server->recv_timeout, sizeof(struct timeval)) < 0)
		std::cerr << RED "Error:" RESET " setsockopt(SO_RCVTIMEO) failed" << std::endl;
	if (setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&_server->send_timeout, sizeof(struct timeval)) < 0)
		std::cerr << RED "Error:" RESET " setsockopt(SO_SNDTIMEO) failed" << std::endl;
}

void	Client::setRecvSize(int size) { _recvSize = size; }

int		Client::getSocket(void) const { return _socket; }

int		Client::getRecvSize(void) const { return _recvSize; }

struct timeval	Client::getTimer(void) const { return _timer; }

std::string	Client::setRootPath(std::string path)
{
	std::size_t	lenght;
	std::string	root;
	int index = -1;

	for (size_t i = 0; i < _server->locations.size(); i++)
	{
		
	}
	
	if (index == -1)
		root = _server->root;
	return (root);
}

// const char	*Client::setClientAddr(void)
// {
// 	if (_addr.ss_family == AF_INET)
// 		return inet_ntoa(((struct sockaddr_in *)&_addr)->sin_addr);
// 	else
// 		return inet_ntoa(((struct sockaddr_in6 *)&_addr)->sin6_addr);
// }

// const char	*Client::setClientPort(void)
// {
// 	if (_addr.ss_family == AF_INET)
// 		return ft_itoa(ntohs(((struct sockaddr_in *)&_addr)->sin_port));
// 	else
// 		return ft_itoa(ntohs(((struct sockaddr_in6 *)&_addr)->sin6_port));
// }

int		Client::charCounter(std::string str, char c)
{
	int count = 0;
	int i = -1;

	while (str[++i])
		count = (str[i] == c) ? count + 1 : count;
	return (count);
}

void	Client::clearRequest(void)
{
	memset(_request, 0, MAX_REQUEST_SIZE + 1);
	_recvSize = 0;
}