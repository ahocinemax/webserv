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
# include "CgiHandler.hpp"
# include "Utils.hpp"

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

		/* EPOLL */
		static void			initEvent(struct epoll_event &event, uint32_t flag, int fd);
		int					connectEpollToSockets(void);
		int					initConnection(int socket);

		void				handleRequest(Client *client, struct epoll_event &event);
		void				handleResponse(Client*, Request*, struct epoll_event&);
		int					findClientIndex(int socket);
		int					routine(void);

		void				editSocket(int socket, uint32_t flag, struct epoll_event event);
		void				removeSocket(int socket);
		void				eraseClient(int index);
		/*  END  */

	private:
		std::vector<Client*>	_clients;
		ServerVector			_serversVec;
		ServerMap				_serversMap;
		ServerMap				_defaultServers;
		StatusMap				_statusCodeList;
		int						_epollFd;

		// Response builder
		int					writeResponse(Client &client, std::string response, std::string path);
		void				sendAutoindex(Client &client, std::string filePath);
		void				redirectMethod(Client &client, Request &request);
		void				deleteMethod(Client &client, std::string path);
		void				postMethod(Client &client, Request &request);
		void				getMethod(Client &client, std::string path);

		// CGI methods
		std::pair<bool, std::vector<std::string> >	isValidCGI(Request &request, Client &client) const;
		bool 				HandleCgi(Request &request, Client& client);
		void				postCgiMethod(Client &client, Request *req);
		void				getCgiMethod(Client &client, Request *req);
		void				eraseTmpFile(StrVector vec);
		bool				isMultipartFormData(Request &request);
		bool				getBoundary(std::string contentType, std::string &boundary);
		size_t				getfield(std::string content, const std::string &field, std::string *name);
		void				handleMultipart(Request &request, Client &client);
		void				setStatusCodes(void);
		std::string			getPath(Client &client, std::string path);
		bool				clientNotConnected(int socket);
		const char			*getMimeType(const char *path);

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