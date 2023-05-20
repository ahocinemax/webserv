/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(Server *server) : _addrLen(sizeof(_addr)), _server(server)
{
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
	std::size_t	len;
	std::string	root;
	int index = -1;

	for (size_t i = 0; i < _server->locations.size(); i++)
	{
		if (_server->locations[i]._root != "" && path.find(_server->locations[i]._path) != std::string::npos)
		{
			if (charCounter(path, '/') == charCounter(_server->locations[i]._path, '/'))
			{
				if (strcmp(strrchr(path.c_str(), '/'), strrchr(_server->locations[i]._path.c_str(), '/')))
					continue;
			}
			if (len < _server->locations[i]._path.length())
			{
				len = _server->locations[i]._root.length();
				root = _server->locations[i]._root;
				index = i;
			}
		}
	}
	
	if (index == -1)
		root = _server->root;
	return (root);
}

const char	*Client::setClientAddr(void)
{
	static char buffer[100];
	getnameinfo((struct sockaddr *)&_addr, _addrLen, buffer, sizeof(buffer), 0, 0, NI_NUMERICHOST);
	return (buffer);
}

const char	*Client::setClientPort(void)
{
	static char buffer[100];
	getnameinfo((struct sockaddr *)&_addr, _addrLen, 0, 0, buffer, sizeof(buffer), NI_NUMERICHOST);
	return (buffer);
}

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
