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

# include <cstdio>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/epoll.h>

# define MAX_EPOLL_EVENTS 1000

class Webserv
{
	public:
		Webserv(ServerVector servers);
		~Webserv(void);

		void				createServers(void);
		void				closeServers(void);

	private:
		std::vector<Client>	_clients;
		ServerVector		_serversVec;
		ServerMap			_serversMap;
		ServerMap			_defaultServers;
		StatusMap			_statutCode;
		int					_maxFd;
		struct epoll_event	_events[MAX_EPOLL_EVENTS];

		void				deleteMethod(Client &client, std::string path);
		void				postMethod(Client &client, Request &request);
		void				getMethod(Client &client, std::string path);

		void				setStatusCodes(void);

		std::string			getPath(Client &client, std::string path);

		int					writeResponse(Client &client, std::string response, std::string path);

};




#endif