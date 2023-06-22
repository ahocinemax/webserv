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

# include "Response.hpp"
# include "Server.hpp"
# include "Request.hpp"
# include "Utils.hpp"

# include <sys/epoll.h>
# include <sys/time.h>
# include <fstream>

class Client
{
	public:
		socklen_t			_addrLen;
		struct sockaddr		_addr;
		Server				*_server;
		const char			*_ipAdress;
		const char			*_port;

		Client(Server *server);
		~Client(void);

		std::string	setRootPath(std::string path);
		void		setRecvSize(int size);
		int			setSocket(int socket);
		void		setTimer(void);

		Request		getRequest(void) const;
		int			getRecvSize(void) const;
		int			getSocket(void) const;
		const char	*getClientAddr(void);
		const char	*getClientPort(void);
		timeval		getTimer(void) const;

		int			charCounter(std::string str, char c);
		void		clearRequest(void);

		void		displayErrorPage(StatusMap::iterator statusCode);
		void		parse(const std::string& str);
		bool		sendContent(const char *content, std::size_t size, bool displayErrorPage = false);

	private:
		int			_socket;
		int			_recvSize;
		timeval		_timer;
		Request		_request;
		Response	_response;
};

#endif