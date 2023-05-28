/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(Server *server) : _addrLen(sizeof(_addr)), _request(0), _server(server)
{
	//clearRequest();
	gettimeofday(&_timer, NULL);
	//memset(_request, 0, MAX_REQUEST_SIZE + 1);
}

Client::~Client(void)
{ 
	if (_request)
		delete _request;
}

void	Client::setTimer(struct timeval &timer) { _timer = timer; }

int	Client::setSocket(int socket)
{
	_socket = socket;
	if (fcntl(_socket, F_SETFL, O_NONBLOCK) < SUCCESS)
	{
		std::cerr << RED "Error:" RESET " fcntl() failed" << std::endl;
		return (FAILED);
	}
	if (setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&_server->recv_timeout, sizeof(struct timeval)) < 0)
	{
		std::cerr << RED "Error:" RESET " setsockopt(SO_RCVTIMEO) failed" << std::endl;
		return (FAILED);
	}
	if (setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&_server->send_timeout, sizeof(struct timeval)) < 0)
	{
		std::cerr << RED "Error:" RESET " setsockopt(SO_SNDTIMEO) failed" << std::endl;
		return (FAILED);
	}
	return (SUCCESS);
}

void	Client::setRecvSize(int size) { _recvSize = size; }

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

const char	*Client::getClientAddr(void)
{
	static char buffer[100];
	getnameinfo((struct sockaddr *)&_addr, _addrLen, buffer, sizeof(buffer), 0, 0, NI_NUMERICHOST);
	return (buffer);
}

const char	*Client::getClientPort(void)
{
	static char buffer[100];
	getnameinfo((struct sockaddr *)&_addr, _addrLen, 0, 0, buffer, sizeof(buffer), NI_NUMERICHOST);
	return (buffer);
}

int		Client::getSocket(void) const { return _socket; }

int		Client::getRecvSize(void) const { return _recvSize; }

struct timeval	Client::getTimer(void) const { return _timer; }

Request	*Client::getRequest() const { return (_request); }

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

void	Client::displayErrorPage(StatusMap::iterator statusCode)
{
	std::cout << RED "> Sending error page: " RESET << statusCode->first << " " << statusCode->second << std::endl;
	std::ifstream file;
	if (statusCode != _server->error_pages.end())
	{
		file.open(_server->error_pages[statusCode->first].c_str());
		if (!file.is_open())
			statusCode = _server->error_pages.find(NOT_FOUND);
	}

	Response response(statusCode->second);

	if (file.is_open()) // Une page d'erreur a été trouvée
	{
		std::string body;
		std::string line;
		while (!file.eof())
		{
			getline(file, line);
			body += line;
			body += "\n";
		}
		response.setCustomizeErrorMessage(body);
		file.close();
	}
	else // Pas de page d'erreur trouvée, page par défaut
		response.setDefaultErrorMessage();
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Content-Length", to_string(response.getBody().length()));
	if (statusCode->first == METHOD_NOT_ALLOWED)
	{
		std::string allowedMethods;
		for (MethodVector::iterator it = _server->allowMethods.begin() ; it != _server->allowMethods.end() ; it++)
		{
			if (it != _server->allowMethods.begin())
				allowedMethods += ", ";
			allowedMethods += methodTypeToStr(*it);
		}
		response.addHeader("Allow", allowedMethods);
	}

	std::string	result = response.makeHeader(true);
	int			sendSize = send(_socket, result.c_str(), result.length(), 0);
	if (sendSize < 0)
		std::cerr << RED "Error:" RESET " send() failed" << std::endl;
	else if (sendSize == 0)
		std::cerr << RED "Error:" RESET " send() failed: connection closed" << std::endl;
	else
		std::cout << GREEN "> Response sent: " RESET << sendSize << " bytes" << std::endl;
	clearRequest();
}

int Client::parse(const std::string& str)
{
	_request = new Request(str);
	int i = _request->parse();
	_request->PrintHeader();
	return i;
}