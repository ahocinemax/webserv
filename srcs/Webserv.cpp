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
#include <sys/types.h>
#include <dirent.h>

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
		std::cout << "> Creating server " << i++ << "(" << it->server_name << 
		"): " CYAN << it->_ipAddress << RESET ":" YELLOW << it->_port << RESET << std::endl;
		it->createSocket();
		_serversMap.insert(std::make_pair(it->_socket, &(*it)));
	}
	connectEpollToSockets();
}

void	Webserv::closeServers(void)
{
	for (ServerMap::iterator it = _serversMap.begin(); it != _serversMap.end(); it++)
	{
		// std::cout << "> Closing server: " << it->first << std::endl;
		close(it->second->_socket);
	}
	// for (int i = 0 ; i < _clients.size() ; i++)
	// {
	// 	eraseClient(i);
	// 	_clients[i]->~Client();
	// }
	for (std::vector<Client*>::iterator it = _clients.begin() ; it != _clients.end() ; it++)
		delete *it;
	_clients.clear();
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
	std::string	tmpPath = path + ".tmp";
	std::string	command = "mkdir -p " + dirPath;
	int			fd = open(tmpPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	system(command.c_str());
	if (fd < 0)
		return (client.displayErrorPage(_statusCodeList.find(500)), FAILED);


	// add fd to epoll
	struct epoll_event	event;
	initEvent(event, EPOLLOUT | EPOLLET, fd);
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) < 0)
		return (client.displayErrorPage(_statusCodeList.find(500)), FAILED);

	// write body to file
	int		ret = write(fd, body.c_str(), body.length());
	// std::cout << GREEN "> Response sent: " RESET << convertToOctets(ret) << "." << std::endl;
	if (ret < 0)
	{
		// close fds
		close(fd);
		epoll_ctl(client.getSocket(), EPOLL_CTL_DEL, fd, &event);
		return (client.displayErrorPage(_statusCodeList.find(500)), FAILED);
	}
	return (close(fd), SUCCESS);
}

void	Webserv::sendAutoindex(Client &client, std::string filePath)
{
	std::string		path = filePath;
	std::string		output = "<html><head><title>Index of " + path + "</title></head><body><h1>Index of " + path + "</h1><hr><pre>";
	DIR				*dir = NULL;
	std::cout << "Autoindexing " << filePath << std::endl;
	if (filePath[filePath.length() - 1] != '/')
		filePath += "/";
	if ((dir = opendir(filePath.c_str())) == NULL)
	{
		client.displayErrorPage(_statusCodeList.find(NOT_FOUND));
		return ;
	}
	struct dirent	*ent;
	filePath.erase(filePath.begin(), filePath.begin() + client._server->root.length());
	while ((ent = readdir(dir)) != NULL)
	{
		if (ent->d_name[0] != '.' || strcmp(ent->d_name, ".."))
			output += "<a href='" + filePath + ent->d_name + "'>";
		else if (filePath[filePath.length() - 1] != '/')
			output += "<a href='" + filePath + ent->d_name + "'>";
		else
			output += "<a href='" + filePath + ent->d_name + "'>";
		output += ((ent->d_type == DT_DIR) ? "" : "./");
		output += (std::string)(ent->d_name) + (ent->d_type == DT_DIR ? "/" : "") + "</a><br>";
	}
	closedir(dir);
	output += "</pre><hr></body></html>";

	Response	response(_statusCodeList[OK]);
	response.addHeader("Content-Type", "text/html");
	response.addHeader("Content-Length", to_string(output.length()));
	std::string	header = response.makeHeader();

	int ret = send(client.getSocket(), header.c_str(), header.length(), MSG_NOSIGNAL);
	if (ret < 0)
	{
		std::cout << RED << "Error while sending header :\n" << header << RESET << std::endl;
		client.displayErrorPage(_statusCodeList.find(INTERNAL_SERVER_ERROR));
	}
	else if (ret == 0)
		client.displayErrorPage(_statusCodeList.find(BAD_REQUEST));
	ret = send(client.getSocket(), output.c_str(), output.length(), MSG_NOSIGNAL);
	if (ret < 0)
		client.displayErrorPage(_statusCodeList.find(INTERNAL_SERVER_ERROR));
	else if (ret == 0)
		client.displayErrorPage(_statusCodeList.find(BAD_REQUEST));
}

void	Webserv::redirectMethod(Client &client, Request &request)
{
	std::cout << BLUE "> Redirecting to " << client._server->redirect_url << RESET << std::endl;
	Response	response(_statusCodeList[client._server->redirect_status]);
	response.addHeader("location", client._server->redirect_url);
	if (!client._server->server_name.empty())
		response.addHeader("server", client._server->server_name);
	response.addHeader("content-type", "text/html");
	response.addHeader("content-length", 0);
	response.addHeader("date", response.getDate());
	std::string	header = response.makeHeader();
	client.sendContent(header.c_str(), header.length());
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
	std::string		filePath = getPath(client, request.getPath());

	struct stat		fileStat;
	lstat(filePath.c_str(), &fileStat);
	if (S_ISDIR(fileStat.st_mode))
	{
		if (request._header.find("content-type") == request._header.end())
			return (client.displayErrorPage(_statusCodeList.find(BAD_REQUEST)));
		std::size_t	begin = request._header["content-type"].find("boundary=");
		if (begin == std::string::npos)
			return (client.displayErrorPage(_statusCodeList.find(BAD_REQUEST)));
		std::string	boundary = request._header["content-type"].substr(begin + 9);
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
				std::cout << request.getBody() << std::endl;
			if (writeResponse(client, request.getBody().substr(begin, end - begin - 4), filePath + "/" + fileName) == FAILED)
				break ;
			if (request.getBody()[end + boundary.length()] == '-')
				break ;
		}		
	}
	else
		writeResponse(client, request.getBody(), filePath);
	int	code = 201;
	if (request._header["content-length"] == "0")
		code = 204;

	Response	response(_statusCodeList[code]);
	std::string	header = response.makeHeader();

	int ret = send(client.getSocket(), header.c_str(), header.length(), MSG_NOSIGNAL);
	if (ret < 0)
		client.displayErrorPage(_statusCodeList.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statusCodeList.find(400));
	else
		std::cout << GREEN << filePath << " posted (" << code << ")" RESET << std::endl;
}

void	Webserv::getMethod(Client &client, std::string path)
{
	std::string		filePath = getPath(client, path);
	if (filePath.length() > MAX_URI_LENGTH)
		return (client.displayErrorPage(_statusCodeList.find(URI_TOO_LONG)));

	struct stat fileStat;
	lstat(filePath.c_str(), &fileStat);
	FILE			*file = fopen(filePath.c_str(), "rb");
	if (file == NULL)
		return (client.displayErrorPage(_statusCodeList.find(NOT_FOUND)));
	fclose(file);

	if (S_ISDIR(fileStat.st_mode))
	{
		std::cout << BLUE "> Path requested is directory" RESET<< std::endl;
		Location *location = client._server->getLocation(path);
		StrVector indexVec;
		bool notFound = true;
		if (location)
			indexVec = location->_index;
		else
			indexVec = client._server->index;

		if (*(filePath.end()) != '/')
			filePath += "/";
		// FIXING filePath = "./html//index.html" :
		if (filePath.find("//") != std::string::npos)
			filePath.erase(filePath.find("//"), 1);
		for (StrVector::iterator it = indexVec.begin(); it != indexVec.end(); it++)
		{
			std::string	tmp = filePath + *it;
			file = fopen(tmp.c_str(), "rb");
			if (file != NULL)
			{
				notFound = false;
				fclose(file);
				filePath += *it;
				break ;
			}
		}

		if (notFound)
		{
			if (client._server->autoindex)
				return (sendAutoindex(client, filePath));
			return (client.displayErrorPage(_statusCodeList.find(NOT_FOUND)));
		}
	}

	file = fopen(filePath.c_str(), "rb");
	const char *mime = getMimeType(filePath.c_str()); // Protected inside getMimeType function
	Response	response(_statusCodeList[client.getRequest()->_statusCode]);
	response.addHeader("content-length", to_string(fileStat.st_size));
	response.addHeader("content-type", mime);
	std::string	header = response.makeHeader(false);
	if (!client.sendContent(header.c_str(), header.length(), true))
		return ;
	char		buffer[BUFFER_SIZE + 1];
	ssize_t		readSize = 0;
	ssize_t		totalSize = header.length();
	while ((readSize = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
	{
		totalSize += readSize;
		if (readSize < 0)
			std::cout << RED "> Error while reading file" RESET << std::endl;
		else if (readSize == 0)
			std::cout << RED "> File is empty" RESET << std::endl;
		else if (!client.sendContent(buffer, readSize))
			break ;
	}
	fclose(file);
	if (totalSize == fileStat.st_size + header.length())
		std::cout << GREEN << filePath << " sent (" << convertToOctets(totalSize) << ")" RESET << std::endl;
	else
	{
		std::cout << RED "> Error on size sent { file_size: " << fileStat.st_size << " ; total sent: ( " << header.length() << " + " << totalSize - header.length() << " )}" RESET << std::endl;
		std::cout << YELLOW "> Status code: " << client.getRequest()->_statusCode << RESET << std::endl;
	}
}

std::pair<bool, std::vector<std::string> > Webserv::isValidCGI(Request &request, Client &client) const
{
	std::pair<bool, std::vector<std::string> > result(false, std::vector<std::string>());
	std::string	path = request.getRoot();
	if (path.length() >= 3)
	{
		std::string extension = path.substr(path.length() - 3, 3);
		if (extension == ".pl" || extension == ".py")
		{
			result.first = true;
			result.second.push_back(path);
			return result;
		}
	}
	if (path.length() >= 4)
	{
		std::string extension = path.substr(path.length() - 4, 4);
		if (extension == ".cgi" || extension == ".pl" || extension == ".py" || extension == ".php")
		{
			result.first = true;
			result.second.push_back(path);
			return result;
		}
	}
	if (path.length() >= 5)
	{
		std::string extension = path.substr(path.length() - 5, 5);
		if (extension == ".html")
		{
			std::ifstream file(path.c_str());
			if (!file)
			{
				std::cerr << RED << "Failed to open file: " << RESET << path << std::endl;
				return result;
			}

			std::ostringstream ss;
			ss << file.rdbuf();
			std::string content = ss.str();
			size_t start = 0;
			size_t end = 0;
			size_t tmp;
			size_t count = 0;
			
			while (start != std::string::npos)
			{
				tmp = end;
				start = content.find("<?php", tmp);
				end = content.find("?>", tmp + 1);
				if (start != std::string::npos && end != std::string::npos)
				{
				std::string phpSection = content.substr(start, end - start + 2); // On ajoute chaque section PHP dans un nouveau fichier
				std::string filePath;
				std::stringstream ss_php;
				ss_php << path << "tmp_" << count++ << ".php"; // Add counter to filename to make it unique
				ss_php >> filePath;
				int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if (fd < 0)
				{
					std::cerr << RED << "Failed to open php script: " << RESET << filePath << std::endl;
					return result;							
				}
				if (write(fd, phpSection.c_str(), phpSection.length()) < 0)
				{
					std::cerr << RED << "Failed to write php script: " << RESET << path << std::endl;
					return result;							
				} // On garde le fichier pour l'exécuter dans le CGI Handler
				if (close(fd) < 0)
				{
					std::cerr << RED << "Failed to close php script: " << RESET << path << std::endl;
					return result;						
				}
				result.first = true;
				result.second.push_back(filePath);
				}
			}
		}
	}
	std::vector<Location>::iterator it = client._server->locations.begin();
	StringMap::iterator it2;
	for (; it != client._server->locations.end(); it++)
	{
		for (it2 = it->_cgi.begin() ; it2 != it->_cgi.end(); it2++)
		{
			if (path.find(it2->first) != std::string::npos)
			{
				if (it2->second != "")
				{
					result.first = true;
					result.second.push_back(path);
					return result;
				}
			}
		}
	}

	return result;
}

void Webserv::postCgiMethod(Client &client, Request *req)
{
	/*
		MEMO: Mariko
		pour l'instant cette fonction est copie-colle de getCgiMethod
		je n'arrive pas a regler des problemes d'execution CGI, je n'ai	pas avance,
		sorry.
		1 parse
		2 check content-type
		3 
		4
		5
		6

	*/
	Response	response(_statusCodeList[client.getRequest()->_statusCode]);
	for (int index = 0 ; index < req->getCgiBody().size() ; index++)
	{
		response.setCgiBody(req->getCgiBody(index));
	}
	std::string		line;
	std::ifstream	file;
	std::size_t		balise;
	line.clear();
	std::string filePath = client._server->root + req->getPath();
	file.open(filePath.c_str(), std::ifstream::in);
	int end;
	int i = 0;
	if (file.is_open())
	{
		while (!file.eof())
		{
			std::getline(file, line);
			balise = line.find("<?php");
			if (balise == std::string::npos)
				response._message.append(line);
			else
			{
				if (balise != 0)
					response._message.append(line, 0, balise - 1);
				response._message.append(response.getCgiBody(i));
				i++;
				while ((end = line.find("?>")) == std::string::npos)
					std::getline(file, line);
				if (end + 2 < line.length())
					response._message.append(line, end + 2, line.length());
			}
		}
		file.close();
	}
	else
		return (client.displayErrorPage(_statusCodeList.find(NOT_FOUND)));
	// remove all 
	response.addHeader("content-length", to_string(response._message.length()));
	// MIME type of CGI script =  "text/html" 
	response.addHeader("content-type", "text/html");
	std::string header = response.makeHeader(false);
	// send header
	if (!client.sendContent(header.c_str(), header.length()))
		return ;
	// send response
	if (!client.sendContent(response._message.c_str(), response._message.length()))
		return ;
	// std::cout << GREEN << "CGI response sent (" << req->_statusCode << ")" RESET << std::endl;
	}

void Webserv::getCgiMethod(Client &client, Request *req)
{
	Response	response(_statusCodeList[client.getRequest()->_statusCode]);
	for (int index = 0 ; index < req->getCgiBody().size() ; index++)
	{
		response.parseCgiBody(req->getCgiBody(index));
		response.setCgiBody(req->getCgiBody(index));
	}
	std::string		line;
	std::ifstream	file;
	std::size_t		balise;
	line.clear();
	std::string filePath = client._server->root + req->getPath();
	file.open(filePath.c_str(), std::ifstream::in);
	int end;
	int i = 0;
	if (file.is_open())
	{
		while (!file.eof())
		{
			std::getline(file, line);
			balise = line.find("<?php");
			if (balise == std::string::npos)
				response._message.append(line);
			else
			{
				if (balise != 0)
					response._message.append(line, 0, balise - 1);
				response._message.append(response.getCgiBody(i));
				i++;
				while ((end = line.find("?>")) == std::string::npos)
					std::getline(file, line);
				if (end + 2< line.length())
					response._message.append(line, end + 2, line.length());
			}
		}
		file.close();
	}
	else
		return (client.displayErrorPage(_statusCodeList.find(NOT_FOUND)));
	response.addHeader("Content-Length", to_string(response._message.length()));
	response.addHeader("Content-Type", "text/html");
	std::string header = response.makeHeader(false);
	if (!client.sendContent(header.c_str(), header.length()))
		return ;
	if (!client.sendContent(response._message.c_str(), response._message.length()))
		return ;
	// std::cout << GREEN << "CGI response sent (" << convertToOctets(header.length() + response._message.length()) << ")" RESET << std::endl;
}

void Webserv::eraseTmpFile(StrVector vec)
{
	for (int i = 0 ; i < vec.size() ; i++)
	{
	    if (vec[i].find("tmp_") != std::string::npos) // prefix pour tmp file dans isValidCGI
        {
            if (remove(vec[i].c_str()) != 0)
                std::cerr << RED << "Failed to remove tmp file: " << RESET << vec[i] << std::endl;
        }
	}
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

bool	Webserv::clientNotConnected(int socket)
{
	for (ServerVector::iterator it = _serversVec.begin(); it != _serversVec.end(); it++)
	{
		if (it->_socket == socket)
			return (false);
	}
	return (true);
}

const char *Webserv::getMimeType(const char *path)
{
	const char *extentionDot = strrchr(path, '.');
	if (extentionDot)
	{
		if (strcmp(extentionDot, ".css") == 0)	return "text/css";
		if (strcmp(extentionDot, ".csv") == 0)	return "text/csv";
		if (strcmp(extentionDot, ".html") == 0) return "text/html";
		if (strcmp(extentionDot, ".js") == 0)	return "application/javascript";
		if (strcmp(extentionDot, ".json") == 0) return "application/json";
		if (strcmp(extentionDot, ".pdf") == 0)	return "application/pdf";
		if (strcmp(extentionDot, ".gif") == 0)	return "image/gif";
		if (strcmp(extentionDot, ".jpeg") == 0) return "image/jpeg";
		if (strcmp(extentionDot, ".jpg") == 0)	return "image/jpeg";
		if (strcmp(extentionDot, ".png") == 0)	return "image/png";
		if (strcmp(extentionDot, ".svg") == 0)	return "image/svg+xml";
	}
	return "text/plain";
}
