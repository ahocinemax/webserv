/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 13:25:29 by ahocine           #+#    #+#             */
/*   Updated: 2023/03/07 05:54:51 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Server.hpp"
#include "Parser.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
		throw std::invalid_argument("Usage: ./webserv <config_file>");

	std::string config_file;

	config_file = argv[1];
	try
	{
		Parser parser(config_file.c_str());
		std::vector<Server> *serv = parser.parse();
		serv->begin()->printInfo();
		delete serv;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}