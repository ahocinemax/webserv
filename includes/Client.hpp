/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/05 19:40:32 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/05 19:43:14 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# define MAX_REQUEST_SIZE 4096
# include "Utils.hpp"
# include "Server.hpp"
# include <sys/time.h>

class Client
{
	public:
		socklen_t					_addrLen;
		struct sockaddr_storage		_addr;
		char						_request[MAX_REQUEST_SIZE];
		Server						*_server;
		int							_epollFd;

		Client(Server *server);
		~Client(void);

		void		setTimer(struct timeval &timer);
		void		setSocket(int socket);
		void		setRecvSize(int size);

		std::string	setRootPath(std::string path);
		const char	*setClientAddr(void);
		const char	*setClientPort(void);

		int			getRecvSize(void) const;
		int			getSocket(void) const;
		timeval		getTimer(void) const;

		int			charCounter(std::string str, char c);
		void		clearRequest(void);

		void		displayErrorPage(StatusMap::iterator statutCode);

	private:
		int		_socket;
		int		_recvSize;
		timeval	_timer;
};

#endif