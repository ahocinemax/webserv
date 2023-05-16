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

# include "Utils.hpp"
# include "Client.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include <cstdio>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/epoll.h>

class Webserv
{
	private:
		std::vector<Server>	_servers;
		std::vector<Client>	_clients;
		StatusMap			_statutCode;

		void	deleteMethod(Client &client, std::string path);
		void	postMethod(Client &client, Request &request);
		void	getMethod(Client &client, std::string path);

		std::string		getPath(Client &client, std::string path);

		int				writeResponse(Client &client, std::string response, std::string path);

	public:
		Webserv(/* args */);
		~Webserv();
};




#endif