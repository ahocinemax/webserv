/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/22 16:06:54 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/22 16:06:56 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location() : _client_body_limit(0), _path(""), _root("")
{
}

Location::~Location(void)
{
}

void	Location::printLocation(void) const
{
	std::cout << "-----------------Location Info: -----------------" << std::endl;
	std::cout << "> path: " << this->_path << std::endl;
	std::cout << "> root: " << this->_root << std::endl;
	std::cout << "> client_body_limit: " << this->_client_body_limit << std::endl;
	std::cout << "> allow_methods: " << std::endl;
	for (unsigned long i = 0; i < _allowMethods.size(); i++)
	{
		switch (_allowMethods[i])
		{
			case GET:
				std::cout << "\tGET" << std::endl;
				break;
			case POST:
				std::cout << "\tPOST" << std::endl;
				break;
			case DELETE:
				std::cout << "\tDELETE" << std::endl;
				break;
			default:
				std::cout << "\tUNKNOWN" << std::endl;
				break;
		}
	}
	std::cout << "> index: " << std::endl;
	for (size_t i = 0; i < this->_index.size(); i++)
		std::cout << "\t" << this->_index[i] << std::endl;
	std::cout << "> cgi_info: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = this->_cgi.begin();
	it != this->_cgi.end(); it++)
		std::cout << "\t" << it->first << ", " << it->second << std::endl;
}

std::string	Location::getRoot(void) const
{
	return (this->_root);
}

std::string	Location::getPath(void) const
{
	return (this->_path);
}

std::string	Location::getCgi(std::string extension) const
{
	for (std::map<std::string, std::string>::const_iterator it = this->_cgi.begin();
	it != this->_cgi.end(); it++)
	{
		if (it->first == "." + extension)
			return (it->second);
	}
	return ("");
}