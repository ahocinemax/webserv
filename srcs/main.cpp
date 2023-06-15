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

#include "Utils.hpp"
#include "Server.hpp"
#include "Parser.hpp"
#include "Webserv.hpp"
#include <csignal>

int g_exit = -42;

void signalHandler(int signum)
{
    if (signum == SIGINT)
	{
        std::cout << "\nSignal SIGINT reçu. Arrêt du serveur." << std::endl;
        g_exit = signum;
    }
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "Usage: ./webserv <config_file>" << std::endl;
		return (FAILED);
	}
	std::string config_file;
	std::signal(SIGINT, signalHandler);
	config_file = argv[1];
	if (config_file.empty())
	{
		std::cout << "Error: Empty config file" << std::endl;
		return (FAILED);
	}
	try
	{
		int i = 1;
		Parser parser(config_file.c_str());
		ServerVector *serv = parser.parse();
		std::cout << "\n> Displaying result ..." << std::endl;
		for (ServerVector::iterator it = serv->begin(); it != serv->end(); it++)
		{
			it->printInfo(i++);
		}
		Webserv webserver(*serv);
		webserver.createServers();
		while (g_exit == -42)
			webserver.routine();
		webserver.closeServers();
		delete serv;
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return (FAILED);
	}
	return (SUCCESS);
}