/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Utils.hpp"

class Request
{
	public:
		Request(/* args */);
		~Request();

		StringMap	_header;

		std::size_t	_contentLength;
		std::string	_method;
		std::string	_path;
		std::string	_body;
};

#endif