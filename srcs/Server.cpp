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

#include "Parser.hpp"

Server::Server(void) :	client_body_limit(1024),
						autoindex(false),
						redirect_status(-1),
						_ipAddress(""),
						_port("")
{
	struct timeval timer;
	timer.tv_sec = 15;
	timer.tv_usec = 0;
	recv_timeout = timer;
	send_timeout = timer;
}

Server::~Server(void)
{
}

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
		std::cout << "> Listening on socket " << newSocket << "..." << std::endl;
		if (listen(newSocket, 100) < 0)
			throw SocketListenException();
		std::cout << std::endl;
		this->_socket = newSocket;
	}
	else
	{
		throw GetAddrInfoException();
	}
}

void	Server::printInfo(int i)
{
	std::cout << PURPLE "-------------  Server " << i << " info:  -------------" RESET << std::endl;
	std::cout << "> Server name: " << server_name << std::endl;
	std::cout << "> IP address: " << _ipAddress << std::endl;
	std::cout << "> Port: " << _port << std::endl;
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
		for (unsigned long i = 0; i < allowMethods.size(); i++)
		{
			switch (allowMethods[i])
			{
				case GET:
					std::cout << "\tGET" << std::endl;
					break;
				case POST:
					std::cout << "\tPOST" << std::endl;
					break;
				case DELETE:
					std::cout << "\tDELETE" << std::endl;
					break;
				default:
					break;
			}
		}
		std::cout << "> send_timeout: " << send_timeout.tv_sec << std::endl;
		std::cout << "> recv_timeout: " << recv_timeout.tv_sec << std::endl;
		std::cout << "> Error pages: ";
		if (error_pages.size() > 0)
		{
			std::cout << std::endl;
			for (StatusMap::iterator it = error_pages.begin(); it != error_pages.end(); it++)
				std::cout << "\t" << it->first << " " << it->second << std::endl;
		}
		else
			std::cout << "None" << std::endl;
		std::cout << "> Locations: " << std::endl;
		for (unsigned long i = 0; i < locations.size(); i++)	
			locations[i].printLocation();
	}
	std::cout << "--------------------------------------------------\n" << std::endl;
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
	if (uriRequest.compare(0, pathLocation.length(), pathLocation) != 0)
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

const char *Server::SocketCreationException::what() const throw() { return ("Error: Cannot create socket"); }

const char *Server::SocketListenException::what() const throw() { return ("Error: Cannot listen to socket"); }

const char *Server::AcceptSocketException::what() const throw() { return ("Error: Server failed to accept incoming connection"); }

const char *Server::SocketConnectionException::what() const throw() { return ("Error: Cannot connect with socket"); }

const char *Server::ReadSocketException::what() const throw() { return ("Error: Cannot read from socket"); }

const char *Server::WriteSocketException::what() const throw() { return ("Error: Cannot write to socket"); }

const char *Server::GetAddrInfoException::what() const throw() { return ("Error: Cannot get address info"); }
