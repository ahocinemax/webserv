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
	gettimeofday(&_timer, NULL);
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
	_ipAdress = getClientAddr();
	_port = getClientPort();
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
	_recvSize = 0;
}

void	Client::displayErrorPage(StatusMap::iterator statusCode)
{
	std::cout << RED "> Sending page: " RESET << statusCode->second << std::endl;
	std::ifstream file;
	if (statusCode != _server->error_pages.end())
	{
		std::string path = _server->error_pages[statusCode->first];
		file.open(path.c_str());
		if (!file.is_open())
			statusCode = _server->error_pages.find(NOT_FOUND);
	}

	Response response(statusCode->second); // Cause de l'erreur de substr

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
		response.setCustomizeStatusPage(body);
		file.close();
	}
	else // page d'erreur pas trouvée, envoie de la page par défaut
		response.setDefaultStatusPage();
	response.addHeader("content-type", "text/html");
	response.addHeader("content-length", to_string(response.getBody().length()));
	if (statusCode->first == METHOD_NOT_ALLOWED)
	{
		std::string allowedMethods;
		for (MethodVector::iterator it = _server->allowMethods.begin() ; it != _server->allowMethods.end() ; it++)
		{
			if (it != _server->allowMethods.begin())
				allowedMethods += ", ";
			allowedMethods += methodTypeToStr(*it);
		}
		response.addHeader("allow", allowedMethods);
	}

	std::string	result = response.makeHeader(true);
	int sendSize = 0;
	if (_socket > 0)
		sendSize = send(_socket, result.c_str(), result.length(), MSG_DONTWAIT | MSG_NOSIGNAL);
	if (sendSize < 0) ;
		// std::cerr << RED "Error:" RESET " send() failed to send error page" << std::endl;
	else if (sendSize == 0)
	{
		std::cerr << RED "Error:" RESET " send() failed: connection closed" << std::endl;
		close(_socket);
	}
	else
		std::cout << GREEN "> Response sent: " RESET << sendSize << " bytes." << std::endl;
}

void Client::parse(const std::string& str)
{
	_request = new Request(str);
	_request->setRoot(_server->root);
	_request->parse();
	// _request->PrintHeader();
	return ;
}

bool	Client::sendContent(const char *content, std::size_t size, bool display)
{
	ssize_t sendSize = 0;
	if (_socket > 0)
		sendSize = send(_socket, content, size, MSG_NOSIGNAL);
	else
		return (printf("send failed\n"), false);
	if (sendSize < 0)
	{
		if (display)
			displayErrorPage(_server->error_pages.find(INTERNAL_SERVER_ERROR));
		return (false);
	}
	else if (sendSize == 0)
	{
		if (display)
			displayErrorPage(_server->error_pages.find(BAD_REQUEST));
		return (false);
	}
	else
		std::cout << "> " GREEN "Response sent: " RESET << sendSize << " bytes." << std::endl;
	return (true);
}
