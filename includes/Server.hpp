/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 13:25:29 by ahocine           #+#    #+#             */
/*   Updated: 2023/03/07 05:54:51 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "webserv.hpp"
# include <sys/socket.h>
# include <netdb.h>
# include <exception>

class Server
{
	public:
		Server(std::string ipAddress, int port);
		~Server(void);

		void	listenSocket();

		// Exceptions
		class SocketCreationException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("error: Cannot create socket");
				}
		};

		class ListenException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("error: Cannot listen to socket");
				}
		};

		class AcceptSocketException : public std::exception
		{
			friend class Server;

			public:
				virtual const char *what() const throw()
				{
					return ("Server failed to accept incoming connection");
				}
		};

		class SocketConnectionException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("error: Cannot connect with socket");
				}
		};

		class ReadSocketException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("error: Cannot read from socket");
				}
		};

		class WriteSocketException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("error: Cannot write to socket");
				}
		};

	private:
		std::string			_ipAddress;
		int					_port;
		int					_socket;
		int					_newSocket;
		long				_incommingMsg;
		struct sockaddr_in	_socketAddr;
		socklen_t			_socketAddrLen;
		std::string			_serverMessage;

		void	startServer();
		void	closeServer();
		void	getSocketAddr();
		void	acceptConnection(int &newSocket);
		char	*readSocket();
		void	buildResponse(char *request);
		void	writeSocket(std::string message);
};

#endif