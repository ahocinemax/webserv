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

# include "Utils.hpp"
# include "Location.hpp"
# include <sys/socket.h>
# include <sys/types.h>
# include <netdb.h>
# include <exception>
# include <fcntl.h>
# include <stdio.h>

class Server
{
	public:
		int						client_body_limit;
		MethodVector			allowMethods;
		StatusMap				error_pages;
		std::string 			server_name;
		bool					autoindex;
		StrVector				index;
		std::string 			root;
		
		std::vector<Location>	locations;

		int						redirect_status;
		std::string				redirect_url;

		struct timeval			send_timeout;
		struct timeval			recv_timeout;

	public:
		std::string				_ipAddress;
		int						_socket;
		std::string				_port;

		~Server(void);
		Server(void);

		void	createSocket(void);
		void	printInfo(int i);

		Location	*getLocation(std::string uriRequest) const;
		bool		validLocation(std::string path, std::string request) const;

		// Exceptions
		class SocketCreationException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("Error: Cannot create socket");
				}
		};

		class SocketListenException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("Error: Cannot listen to socket");
				}
		};

		class AcceptSocketException : public std::exception
		{
			friend class Server;

			public:
				virtual const char *what() const throw()
				{
					return ("Error: Server failed to accept incoming connection");
				}
		};

		class SocketConnectionException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("Error: Cannot connect with socket");
				}
		};

		class ReadSocketException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("Error: Cannot read from socket");
				}
		};

		class WriteSocketException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("Error: Cannot write to socket");
				}
		};

		class GetAddrInfoException : public std::exception
		{
			public:
				virtual const char *what() const throw()
				{
					return ("Error: Cannot get address info");
				}
		};

};

#endif
