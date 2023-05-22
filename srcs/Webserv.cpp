#include "Webserv.hpp"

Webserv::Webserv(ServerVector server) : _serversVec(server), _maxFd(-1)
{
	setStatusCodes();
	(void)_maxFd;
	for (unsigned int i = 0 ; i < _serversVec.size() ; i++)
	{
		for (StatusMap::iterator it = _serversVec[i].error_pages.begin() ; it != _serversVec[i].error_pages.end() ; it++)
		{
			int statusCode = it->first;
			if (statusCode < 400 || (statusCode > 429 && statusCode < 500) || statusCode > 505)
				std::cout << RED "Error: Invalid status code \"" << statusCode << "\" in server \"" << _serversVec[i].server_name << "\"" RESET << std::endl;
		}
		if (!_defaultServers[_serversVec[i]._port])
			_defaultServers[_serversVec[i]._port] = &_serversVec[i];
		
		if (!_serversMap[_serversVec[i].server_name + ":" + _serversVec[i]._port])
			_serversMap[_serversVec[i].server_name + ":" + _serversVec[i]._port] = &_serversVec[i];
		else
			std::cout << RED "Error: Server \"" << _serversVec[i].server_name << "\" already exists" RESET << std::endl;
	}
}

Webserv::~Webserv() {}

void Webserv::createServers(void)
{
	for (ServerMap::iterator it = _defaultServers.begin() ; it != _defaultServers.end() ; it ++)
	{
		std::cout << "> Creating server: " << it->first << std::endl;
		it->second->createSocket();
	}
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
		{
			client.displayErrorPage(_statutCode.find(400));
			return ;
		}
		std::size_t	begin = request._header["Content-Type"].find("boundary=");
		if (begin == std::string::npos)
		{
			client.displayErrorPage(_statutCode.find(400));
			return ;
		}
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
	{
		writeResponse(client, request._body, filePath);
	}
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
	{
		client.displayErrorPage(_statutCode.find(404));
		return ;
	}
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
	_statutCode[200] = "200 OK";
	_statutCode[201] = "201 Created";
	_statutCode[202] = "202 Accepted";
	_statutCode[204] = "204 No Content";
	_statutCode[301] = "301 Moved Permanently";
	_statutCode[302] = "302 Found";
	_statutCode[303] = "303 See Other";
	_statutCode[304] = "304 Not Modified";
	_statutCode[307] = "307 Temporary Redirect";
	_statutCode[308] = "308 Permanent Redirect";
	_statutCode[400] = "400 Bad Request";
	_statutCode[401] = "401 Unauthorized";
	_statutCode[403] = "403 Forbidden";
	_statutCode[404] = "404 Not Found";
	_statutCode[405] = "405 Method Not Allowed";
	_statutCode[406] = "406 Not Acceptable";
	_statutCode[408] = "408 Request Timeout";
	_statutCode[409] = "409 Conflict";
	_statutCode[410] = "410 Gone";
	_statutCode[411] = "411 Length Required";
	_statutCode[413] = "413 Payload Too Large";
	_statutCode[414] = "414 URI Too Long";
	_statutCode[415] = "415 Unsupported Media Type";
	_statutCode[416] = "416 Range Not Satisfiable";
	_statutCode[417] = "417 Expectation Failed";
	_statutCode[429] = "429 Too Many Requests";
	_statutCode[500] = "500 Internal Server Error";
	_statutCode[501] = "501 Not Implemented";
	_statutCode[502] = "502 Bad Gateway";
	_statutCode[503] = "503 Service Unavailable";
	_statutCode[504] = "504 Gateway Timeout";
	_statutCode[505] = "505 HTTP Version Not Supported";
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
		{
			client.displayErrorPage(_statutCode.find(400));
			return (FAILED);
		}
	}
	std::string	command = "mkdir -p " + dirPath;
	int			fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	system(command.c_str());
	if (fd < 0)
	{
		client.displayErrorPage(_statutCode.find(500));
		return (FAILED);
	}

	// add fd to epoll
	struct epoll_event	event;
	event.events = EPOLLOUT | EPOLLET;
	event.data.fd = fd;
	if (epoll_ctl(client._epollFd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		client.displayErrorPage(_statutCode.find(500));
		return (FAILED);
	}

	// write body to file
	int		ret = write(fd, body.c_str(), body.length());
	if (ret < 0)
	{
		// close fds
		close(fd);
		epoll_ctl(client._epollFd, EPOLL_CTL_DEL, fd, &event);
		client.displayErrorPage(_statutCode.find(500));
		return (FAILED);
	}
	close(fd);
	return (SUCCESS);
}
