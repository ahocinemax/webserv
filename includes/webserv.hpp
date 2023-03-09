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

# include <iostream>
# include <string>
# include <vector>
# include <map>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>

# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"
# include "Location.hpp"
# include "Client.hpp"

# define BUFFER_SIZE 30720

enum MethodTypes
{
	OPTIONS,
	GET,
	HEAD,
	POST,
	PUT,
	DELETE,
	TRACE,
	CONNECT
};

// struct sockaddr_in
// {
// 	short			sin_family;
// 	int				sin_type;
// 	unsigned short	sin_port;
// 	struct in_addr	sin_addr;
// 	char			sin_zero[8];
// };

// struct in_addr
// {
// 	unsigned long	s_addr;
// };

#endif