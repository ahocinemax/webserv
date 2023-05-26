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
		if (_defaultServers.find(_serversVec[i]._socket) == _defaultServers.end())
			_defaultServers[_serversVec[i]._socket] = &_serversVec[i];

		if (_serversMap.find(_serversVec[i]._socket))
			_serversMap[_serversVec[i]._socket] = &_serversVec[i];
		else
			std::cout << RED "Error: Server \"" << _serversVec[i].server_name << "\" already exists" RESET << std::endl;
	}
}

Webserv::~Webserv() {}

void Webserv::createServers(void)
{
	for (ServerMap::iterator it = _defaultServers.begin() ; it != _defaultServers.end() ; it ++)
	{
		std::cout << "> Creating server: " BLUE << it->second->_ipAddress << RESET << std::endl;
		it->second->createSocket();
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
		client.displayErrorPage(_statutCode.find(404));
		return ;
	}
	fclose(file);
	
	std::remove(filePath.c_str());

	Response	response(_statutCode[200]);
	std::string	header = response.makeHeader();
	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);

	if (ret < 0)
		client.displayErrorPage(_statutCode.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statutCode.find(400));
	std::cout << GREEN "File \"" << filePath << "\" deleted" RESET << std::endl;
}

void	Webserv::postMethod(Client &client, Request &request)
{
	if (request._header["Transfer-Encoding"] != "chunked" && request._header.find("Content_Lenght") == request._header.end())
	{
		client.displayErrorPage(_statutCode.find(411));
		return ;
	}

	std::string		filePath = getPath(client, request._path);

	struct stat		fileStat;
	lstat(filePath.c_str(), &fileStat);
	if (S_ISDIR(fileStat.st_mode))
	{
		if (request._header.find("Content-Type") == request._header.end())
			return (client.displayErrorPage(_statutCode.find(400)));
		std::size_t	begin = request._header["Content-Type"].find("boundary=");
		if (begin == std::string::npos)
			return (client.displayErrorPage(_statutCode.find(400)));
		std::string	boundary = request._header["Content-Type"].substr(begin + 9);
		begin = 0;
		std::size_t	end = 0;
		std::string	fileName;
		while (true)
		{
			begin = request._body.find("name=", begin) + 6;
			end = request._body.find_first_of("\"", begin);
			if (begin == std::string::npos || end == std::string::npos)
				break ;
			fileName = request._body.substr(begin, end - begin);
			begin = request._body.find("\r\n\r\n", begin) + 4;
			end = request._body.find(boundary, begin);
			if (begin == std::string::npos || end == std::string::npos)
				break ;
			if (writeResponse(client, request._body.substr(begin, end - begin - 4), filePath + "/" + fileName) == FAILED)
				break ;
			if (request._body[end + boundary.length()] == '-')
				break ;
		}		
	}
	else
		writeResponse(client, request._body, filePath);
	int	code = 201;
	if (request._header["Content-Lenght"] == "0")
		code = 204;
	
	Response	response(_statutCode[code]);
	std::string	header = response.makeHeader();

	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);
	if (ret < 0)
		client.displayErrorPage(_statutCode.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statutCode.find(400));
	std::cout << GREEN << filePath << " posted (" << code << ")" RESET << std::endl;
}

void	Webserv::getMethod(Client &client, std::string path)
{
	std::string		filePath = getPath(client, path);
	FILE			*file = fopen(filePath.c_str(), "r");

	if (file == NULL)
		return (client.displayErrorPage(_statutCode.find(404)));
	fclose(file);

	Response	response(_statutCode[200]);
	std::string	header = response.makeHeader(true);
	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);

	if (ret < 0)
		client.displayErrorPage(_statutCode.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statutCode.find(400));
	std::cout << GREEN << filePath << " sent (" << 200 << ")" RESET << std::endl;
}

void Webserv::setStatusCodes(void)
{
	_statutCode[OK] = "200 OK";
	_statutCode[CREATED] = "201 Created";
	_statutCode[ACCEPTED] = "202 Accepted";
	_statutCode[NO_CONTENT] = "204 No Content";
	_statutCode[MOVED_PERMANENTLY] = "301 Moved Permanently";
	_statutCode[FOUND] = "302 Found";
	_statutCode[SEE_OTHER] = "303 See Other";
	_statutCode[NOT_MODIFIED] = "304 Not Modified";
	_statutCode[TEMPORARY_REDIRECT] = "307 Temporary Redirect";
	_statutCode[PERMANENT_REDIRECT] = "308 Permanent Redirect";
	_statutCode[BAD_REQUEST] = "400 Bad Request";
	_statutCode[UNAUTHORIZED] = "401 Unauthorized";
	_statutCode[FORBIDDEN] = "403 Forbidden";
	_statutCode[NOT_FOUND] = "404 Not Found";
	_statutCode[METHOD_NOT_ALLOWED] = "405 Method Not Allowed";
	_statutCode[NOT_ACCEPTABLE] = "406 Not Acceptable";
	_statutCode[REQUEST_TIMEOUT] = "408 Request Timeout";
	_statutCode[CONFLICT] = "409 Conflict";
	_statutCode[GONE] = "410 Gone";
	_statutCode[LENGTH_REQUIRED] = "411 Length Required";
	_statutCode[PAYLOAD_TOO_LARGE] = "413 Payload Too Large";
	_statutCode[URI_TOO_LONG] = "414 URI Too Long";
	_statutCode[UNSUPPORTED_MEDIA_TYPE] = "415 Unsupported Media Type";
	_statutCode[RANGE_NOT_SATISFIABLE] = "416 Range Not Satisfiable";
	_statutCode[EXPECTATION_FAILED] = "417 Expectation Failed";
	_statutCode[TOO_MANY_REQUESTS] = "429 Too Many Requests";
	_statutCode[INTERNAL_SERVER_ERROR] = "500 Internal Server Error";
	_statutCode[NOT_IMPLEMENTED] = "501 Not Implemented";
	_statutCode[BAD_GATEWAY] = "502 Bad Gateway";
	_statutCode[SERVICE_UNAVAILABLE] = "503 Service Unavailable";
	_statutCode[GATEWAY_TIMEOUT] = "504 Gateway Timeout";
	_statutCode[HTTP_VERSION_NOT_SUPPORTED] = "505 HTTP Version Not Supported";
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
			return (client.displayErrorPage(_statutCode.find(400)), FAILED);
	}
	std::string	command = "mkdir -p " + dirPath;
	int			fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	system(command.c_str());
	if (fd < 0)
		return (client.displayErrorPage(_statutCode.find(500)), FAILED);

	// add fd to epoll
	struct epoll_event	event;
	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = fd;
	if (epoll_ctl(client.getSocket(), EPOLL_CTL_ADD, fd, &event) < 0)
		return (client.displayErrorPage(_statutCode.find(500)), FAILED);

	// write body to file
	int		ret = write(fd, body.c_str(), body.length());
	if (ret < 0)
	{
		// close fds
		close(fd);
		epoll_ctl(client.getSocket(), EPOLL_CTL_DEL, fd, &event);
		return (client.displayErrorPage(_statutCode.find(500)), FAILED);
	}
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
