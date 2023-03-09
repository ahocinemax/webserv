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

namespace
{
	void	log(const std::string &message)
	{
		std::cout << message << std::endl;
	}
}

Server::Server(std::string ipAddress, int port) : _ipAddress(ipAddress),
												  _port(port),
												  _socket(),
												  _newSocket(),
												  _incommingMsg(),
												  _socketAddr(),
												  _socketAddrLen(sizeof(_socketAddr)),
												  _serverMessage()
{
	std::cout << "Starting server ..." << std::endl;
	Server::getSocketAddr();

	try
	{
		Server::startServer();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		exit(1);
	}
	
}

Server::~Server(void) { Server::closeServer(); }

void	Server::listenSocket()
{
	char	*request;
	int		recievedBytes;

	if (listen(_newSocket, 10) < 0)
		throw ListenException();
	std::cout << "Listening to address: " << _ipAddress << " on port: " << _port << std::endl;

	while (42)
	{
		acceptConnection(_newSocket);

		request = readSocket();
		buildResponse(request);
		writeSocket(_serverMessage);
		close(_newSocket);
	}
}

void	Server::startServer()
{
	_socket = socket(_socketAddr.sin_family, SOCK_STREAM, 0);
	std::cout << _socket << "    " << _socketAddr.sin_family << std::endl;
	if (_socket < 0)
		throw SocketConnectionException();
	if (bind(_socket, (struct sockaddr *)&_socketAddr, _socketAddrLen) < 0)
		throw SocketCreationException();
}

void	Server::closeServer() { close(_socket); close(_newSocket); exit(0); }

void	Server::getSocketAddr()
{
	_socketAddr.sin_family = AF_INET;
	_socketAddr.sin_port = htons(_port);
	_socketAddr.sin_addr.s_addr = inet_addr(_ipAddress.c_str());
}

void	Server::acceptConnection(int &new_socket)
{
	new_socket = accept(_socket, (struct sockaddr *)&_socketAddr, &_socketAddrLen);
	if (_newSocket < 0)
		throw AcceptSocketException();
}

char	*Server::readSocket()
{
	char *buffer[BUFFER_SIZE] = {0};
	int valread = read(_newSocket, *buffer, BUFFER_SIZE);
	if (valread < 0)
		throw ReadSocketException();
	std::cout << "Request: " << buffer << std::endl;
	*buffer[valread] = '\0';
	return (*buffer);
}

void	Server::buildResponse(char *request)
{
	// Parse request
	// Build response
	// Set response
	_serverMessage = "HTTP/1.1 200 OK\r";
}

void	Server::writeSocket(std::string message)
{
	long bytesSent;

	bytesSent = write(_newSocket, _serverMessage.c_str(), _serverMessage.size());
	if (bytesSent != _serverMessage.size())
		throw WriteSocketException();
}
