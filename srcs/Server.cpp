/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 13:25:29 by ahocine           #+#    #+#             */
/*   Updated: 2023/03/07 05:54:51 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(void)
{
	client_body_limit = 1024;

	struct timeval timer;
	timer.tv_sec = 60;
	timer.tv_usec = 0;

	if (recv_timeout.tv_sec != 0)	recv_timeout = timer;
	if (send_timeout.tv_sec != 0)	send_timeout = timer;

	autoindex = false;
	_ipAddress = "";
	_port = "";
	redirect_status = -1;
}

Server::~Server(void) {}

void	Server::createSocket(void)
{
	struct addrinfo hints, *info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(_ipAddress.c_str(), _port.c_str(), &hints, &info) >= 0)
	{
		std::cout << "> Creating socket ..." << std::endl;
		int newSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
		fcntl(newSocket, F_SETFL, O_NONBLOCK);
		if (newSocket < 0)
			throw SocketCreationException();
		std::cout << "> Binding socket to local address..." << std::endl;
		int opt = 1;
		if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) < 0)
			std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
		if (bind(newSocket, info->ai_addr, info->ai_addrlen) < 0)
		{
			perror("bind");
			throw SocketConnectionException();
		}
		freeaddrinfo(info);
		std::cout << "> Listening on socket..." << std::endl;
		if (listen(newSocket, 100) < 0)
			throw SocketListenException();
		this->_socket = newSocket;
	}
	else
	{
		throw GetAddrInfoException();
	}
}

void	Server::printInfo(void)
{
	std::cout << "-------------  Server info:  -------------" << std::endl;
	std::cout << "> Server name: " << server_name << std::endl;
	std::cout << "> IP address: " << _ipAddress << std::endl;
	std::cout << "> Port" << _port << std::endl;
	if (redirect_status != -1)
	{
		std::cout << "> Redirect status: " << redirect_status << std::endl;
		std::cout << "> Redirect url: " << redirect_url << std::endl;
	}
	else
	{
		std::cout << "> Root: " << root << std::endl;
		std::cout << "> Client body limit: " << client_body_limit << std::endl;
		std::cout << "> Autoindex: " << autoindex << std::endl;
		std::cout << "> Index: " << std::endl;
		for (unsigned long i = 0; i < index.size(); i++)
			std::cout << "\t" << index[i] << std::endl;
		std::cout << "> Allowed methods: " << std::endl;
		for (unsigned long i = 0; i < allow_methods.size(); i++)
		{
			switch (allow_methods[i])
			{
				case GET:
					std::cout << "\tGET" << std::endl;
					break;
				case POST:
					std::cout << "\tPOST" << std::endl;
					break;
				case PUT:
					std::cout << "\tPUT" << std::endl;
					break;
				case DELETE:
					std::cout << "\tDELETE" << std::endl;
					break;
				case HEAD:
					std::cout << "\tHEAD" << std::endl;
					break;
				case OPTIONS:
					std::cout << "\tOPTIONS" << std::endl;
					break;
				case TRACE:
					std::cout << "\tTRACE" << std::endl;
					break;
				case CONNECT:
					std::cout << "\tCONNECT" << std::endl;
					break;
				default:
					break;
			}
		}
		std::cout << "> Error pages: ";
		if (error_pages.size() > 0)
			std::cout << std::endl;
		else
			std::cout << "None" << std::endl;
		std::cout << "> Locations: " << std::endl;
		for (unsigned long i = 0; i < locations.size(); i++)	
			locations[i].printLocation();
	}
}

MethodType	Server::methodType(std::string uriRequest)
{
	if (uriRequest == "GET")
		return GET;
	else if (uriRequest == "POST")
		return POST;
	else if (uriRequest == "PUT")
		return PUT;
	else if (uriRequest == "DELETE")
		return DELETE;
	else if (uriRequest == "HEAD")
		return HEAD;
	else if (uriRequest == "OPTIONS")
		return OPTIONS;
	else if (uriRequest == "TRACE")
		return TRACE;
	else if (uriRequest == "CONNECT")
		return CONNECT;
	else
		return UNKNOWN;
}

Location	*Server::getLocation(std::string uriRequest) const
{
	if (this->locations.size() == 0)
		return NULL;

	std::vector<Location>::const_iterator res = this->locations.begin();
	unsigned long	longest = 0;
	bool			found = false;

	for (std::vector<Location>::const_iterator it = res; it != locations.end(); it++)
	{
		std::string pathLocation = it->_path;
		if (validLocation(pathLocation, uriRequest) && longest < pathLocation.length())
		{
			res = it;
			longest = pathLocation.length();
			found = true;
		}
	}
	if (found)
		return (const_cast<Location *>(&*res));
	return (NULL);
}

bool	Server::validLocation(std::string pathLocation, std::string uriRequest) const
{
	if (pathLocation.compare(0, uriRequest.length(), uriRequest) != 0)
		return (false);
	else if (uriRequest[pathLocation.length()])
	{
		if (uriRequest[pathLocation.length()] == '/' || \
			uriRequest[pathLocation.length()] == '?')
			return (true);
		else
			return (false);
	}
	return (true);
}