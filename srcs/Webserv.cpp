/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/22 16:06:54 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/22 16:06:56 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "Epoll.hpp"

Webserv::Webserv(ServerVector server) : _serversVec(server)
{
	setStatusCodes();
	for (unsigned int i = 0 ; i < _serversVec.size() ; i++)
	{
		for (StatusMap::iterator it = _serversVec[i].error_pages.begin() ; \
		it != _serversVec[i].error_pages.end() ; it++)
		{
			int statusCode = it->first;
			if (statusCode < BAD_REQUEST || \
			(statusCode > TOO_MANY_REQUESTS &&  statusCode < INTERNAL_SERVER_ERROR) || \
			statusCode > HTTP_VERSION_NOT_SUPPORTED)
			{
				std::cout << RED "Error: Invalid status code \"" << \
				statusCode << "\" in server \"" << _serversVec[i].server_name << "\"" RESET << std::endl;
			}
		}
		// if (_defaultServers.find(_serversVec[i]._socket) == _defaultServers.end())
		// 	_defaultServers.insert(std::make_pair(_serversVec[i]._socket, &_serversVec[i]));

		// if (_serversMap.find(_serversVec[i]._socket) != _serversMap.end())
		// 	_serversMap.insert(std::make_pair(_serversVec[i]._socket, &_serversVec[i]));
	}
}

Webserv::~Webserv() {}

void Webserv::createServers(void)
{
	int i = 1;
	for (ServerVector::iterator it = _serversVec.begin() ; it != _serversVec.end() ; it ++)
	{
		std::cout << "> Creating server " << i++ << BLUE ": " << it->_ipAddress << RESET << std::endl;
		it->createSocket();
		_serversMap.insert(std::make_pair(it->_socket, &(*it)));
	}
	connectEpollToSockets();
}

void	Webserv::closeServers(void)
{
	for (ServerMap::iterator it = _defaultServers.begin(); it != _defaultServers.end(); it++)
	{
		std::cout << "> Closing server: " << it->first << std::endl;
		close(it->second->_socket);
	}
}

void	Webserv::deleteMethod(Client &client, std::string path)
{
	std::string		filePath = getPath(client, path);
	FILE			*file = fopen(filePath.c_str(), "r");

	if (file == NULL)
	{
		client.displayErrorPage(_statusCodeList.find(404));
		return ;
	}
	fclose(file);
	std::remove(filePath.c_str());

	Response	response(_statusCodeList[200]);
	std::string	header = response.makeHeader();
	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);

	if (ret < 0)
		client.displayErrorPage(_statusCodeList.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statusCodeList.find(400));
	std::cout << GREEN "File \"" << filePath << "\" deleted" RESET << std::endl;
}

void	Webserv::postMethod(Client &client, Request &request)
{
	if (request._header["Transfer-Encoding"] != "chunked" && request._header.find("Content_Lenght") == request._header.end())
	{
		client.displayErrorPage(_statusCodeList.find(411));
		return ;
	}

	std::string		filePath = getPath(client, request.getPath());

	struct stat		fileStat;
	lstat(filePath.c_str(), &fileStat);
	if (S_ISDIR(fileStat.st_mode))
	{
		if (request._header.find("Content-Type") == request._header.end())
			return (client.displayErrorPage(_statusCodeList.find(400)));
		std::size_t	begin = request._header["Content-Type"].find("boundary=");
		if (begin == std::string::npos)
			return (client.displayErrorPage(_statusCodeList.find(400)));
		std::string	boundary = request._header["Content-Type"].substr(begin + 9);
		begin = 0;
		std::size_t	end = 0;
		std::string	fileName;
		while (true)
		{
			begin = request.getBody().find("name=", begin) + 6;
			end = request.getBody().find_first_of("\"", begin);
			if (begin == std::string::npos || end == std::string::npos)
				break ;
			fileName = request.getBody().substr(begin, end - begin);
			begin = request.getBody().find("\r\n\r\n", begin) + 4;
			end = request.getBody().find(boundary, begin);
			if (begin == std::string::npos || end == std::string::npos)
				break ;
			if (writeResponse(client, request.getBody().substr(begin, end - begin - 4), filePath + "/" + fileName) == FAILED)
				break ;
			if (request.getBody()[end + boundary.length()] == '-')
				break ;
		}		
	}
	else
		writeResponse(client, request.getBody(), filePath);
	int	code = 201;
	if (request._header["Content-Lenght"] == "0")
		code = 204;
	
	Response	response(_statusCodeList[code]);
	std::string	header = response.makeHeader();

	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);
	if (ret < 0)
		client.displayErrorPage(_statusCodeList.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statusCodeList.find(400));
	std::cout << GREEN << filePath << " posted (" << code << ")" RESET << std::endl;
}

void	Webserv::getMethod(Client &client, std::string path)
{
	std::string		filePath = getPath(client, path);
	FILE			*file = fopen(filePath.c_str(), "r");

	if (file == NULL)
		return (client.displayErrorPage(_statusCodeList.find(404)));
	fclose(file);

	Response	response(_statusCodeList[200]);
	std::string	header = response.makeHeader(true);
	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);

	if (ret < 0)
		client.displayErrorPage(_statusCodeList.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statusCodeList.find(400));
	std::cout << GREEN << filePath << " sent (" << 200 << ")" RESET << std::endl;
}

void Webserv::setStatusCodes(void)
{
	_statusCodeList[OK] = "200 OK";
	_statusCodeList[CREATED] = "201 Created";
	_statusCodeList[ACCEPTED] = "202 Accepted";
	_statusCodeList[NO_CONTENT] = "204 No Content";
	_statusCodeList[MOVED_PERMANENTLY] = "301 Moved Permanently";
	_statusCodeList[FOUND] = "302 Found";
	_statusCodeList[SEE_OTHER] = "303 See Other";
	_statusCodeList[NOT_MODIFIED] = "304 Not Modified";
	_statusCodeList[TEMPORARY_REDIRECT] = "307 Temporary Redirect";
	_statusCodeList[PERMANENT_REDIRECT] = "308 Permanent Redirect";
	_statusCodeList[BAD_REQUEST] = "400 Bad Request";
	_statusCodeList[UNAUTHORIZED] = "401 Unauthorized";
	_statusCodeList[FORBIDDEN] = "403 Forbidden";
	_statusCodeList[NOT_FOUND] = "404 Not Found";
	_statusCodeList[METHOD_NOT_ALLOWED] = "405 Method Not Allowed";
	_statusCodeList[NOT_ACCEPTABLE] = "406 Not Acceptable";
	_statusCodeList[REQUEST_TIMEOUT] = "408 Request Timeout";
	_statusCodeList[CONFLICT] = "409 Conflict";
	_statusCodeList[GONE] = "410 Gone";
	_statusCodeList[LENGTH_REQUIRED] = "411 Length Required";
	_statusCodeList[PAYLOAD_TOO_LARGE] = "413 Payload Too Large";
	_statusCodeList[URI_TOO_LONG] = "414 URI Too Long";
	_statusCodeList[UNSUPPORTED_MEDIA_TYPE] = "415 Unsupported Media Type";
	_statusCodeList[RANGE_NOT_SATISFIABLE] = "416 Range Not Satisfiable";
	_statusCodeList[EXPECTATION_FAILED] = "417 Expectation Failed";
	_statusCodeList[TOO_MANY_REQUESTS] = "429 Too Many Requests";
	_statusCodeList[INTERNAL_SERVER_ERROR] = "500 Internal Server Error";
	_statusCodeList[NOT_IMPLEMENTED] = "501 Not Implemented";
	_statusCodeList[BAD_GATEWAY] = "502 Bad Gateway";
	_statusCodeList[SERVICE_UNAVAILABLE] = "503 Service Unavailable";
	_statusCodeList[GATEWAY_TIMEOUT] = "504 Gateway Timeout";
	_statusCodeList[HTTP_VERSION_NOT_SUPPORTED] = "505 HTTP Version Not Supported";
}

std::string	Webserv::getPath(Client &client, std::string path)
{
	std::string		filePath = "";
	std::string		res = "";

	filePath.append(client.setRootPath(path));
	Location	*location = client._server->getLocation(path);
	if (location != NULL)
		res = location->getPath();
	filePath.append(path.substr(res.length()));
	return (filePath);	
}

int	Webserv::writeResponse(Client &client, std::string body, std::string path)
{
	std::size_t	begin = path.find_last_of("/");
	std::string	dirPath = path.substr(0, begin);
	std::string	fileName = path.substr(begin + 1);

	if (dirPath != "")
	{
		struct stat		fileStat;
		lstat(dirPath.c_str(), &fileStat);
		if (!S_ISDIR(fileStat.st_mode))
			return (client.displayErrorPage(_statusCodeList.find(400)), FAILED);
	}
	std::string	command = "mkdir -p " + dirPath;
	int			fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	system(command.c_str());
	if (fd < 0)
		return (client.displayErrorPage(_statusCodeList.find(500)), FAILED);

	// add fd to epoll
	struct epoll_event	event;
	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = fd;
	if (epoll_ctl(client.getSocket(), EPOLL_CTL_ADD, fd, &event) < 0)
		return (client.displayErrorPage(_statusCodeList.find(500)), FAILED);

	// write body to file
	int		ret = write(fd, body.c_str(), body.length());
	if (ret < 0)
	{
		// close fds
		close(fd);
		epoll_ctl(client.getSocket(), EPOLL_CTL_DEL, fd, &event);
		std::cout << "epoll_ctl error(4) : " << errno << std::endl;
		return (client.displayErrorPage(_statusCodeList.find(500)), FAILED);
	}
	std::cout << "close" << std::endl;
	return (close(fd), SUCCESS);
}

bool	Webserv::clientNotConnected(int socket)
{
	for (ServerVector::iterator it = _serversVec.begin(); it != _serversVec.end(); it++)
	{
		if (it->_socket == socket)
			return (false);
	}
	return (true);
}
