/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 13:25:29 by ahocine           #+#    #+#             */
/*   Updated: 2023/03/07 05:54:51 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "Response.hpp"
# include "Request.hpp"
# include "Client.hpp"

# include <sys/epoll.h>
# include <sys/stat.h>
# include <unistd.h>
# include <cstdio>
# include <limits>
# include <iomanip>

# define MAX_EPOLL_EVENTS 1000

class Webserv
{
	public:
		Webserv(ServerVector servers);
		~Webserv(void);

		void				createServers(void);
		void				closeServers(void);
		int					routine(void);

		/* EPOLL */
		static void			initEvent(struct epoll_event &event, uint32_t flag, int fd);
		int					initConnection(int socket);
		int					connectEpollToSockets(void);
		void		        initEpoll(void);
		int					findClientIndex(int socket);

		void				handleRequest(Client &client);
		void				handleResponse(Client &client);
		bool				clientNotConnected(int socket);

	private:
		std::vector<Client>	_clients;
		ServerVector		_serversVec;
		ServerMap			_serversMap;
		ServerMap			_defaultServers;
		StatusMap			_statusCodeList;
		int					_epollFd;

		void				deleteMethod(Client &client, std::string path);
		void				postMethod(Client &client, Request &request);
		void				getMethod(Client &client, std::string path);

		void				setStatusCodes(void);

		std::string			getPath(Client &client, std::string path);

		int					writeResponse(Client &client, std::string response, std::string path);

		class EpollCreateException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class EpollCtlException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class EpollWaitException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};

		class AcceptException : public std::exception
		{
			public:
				virtual const char* what() const throw();
		};
};




#endif