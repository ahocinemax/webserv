/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 13:25:29 by ahocine           #+#    #+#             */
/*   Updated: 2023/03/07 05:54:51 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

Parser::Parser(const char *config_file)
{
	std::string     line;
	std::ifstream   fs;

	line.clear();
	fs.open(config_file);
	if (fs.is_open())
	{
		while (!fs.eof())
		{
			std::getline(fs, line);
			_content.append(line + '\n');
		}
		fs.close();
	}
	else
	{
		std::cerr << "Error: Cannot open config file" << std::endl;
		exit(1);
	}
}

Parser::~Parser(void) {}

std::vector<Server>	*Parser::parse(void)
{
	std::vector<Server> *result = new std::vector<Server>();

	std::cout << "> Parsing config file ..." << std::endl;
	std::size_t prev = 0;
	std::size_t curr = _content.find_first_not_of(" \t\n", prev);

	if (curr == std::string::npos)
		exit(printError());
	while (curr != std::string::npos)
	{
		prev = _content.find_first_not_of(' \n\t', curr);
		curr = _content.find_first_of(' \n\t', prev);
		std::string key = _content.substr(prev, curr - prev);
		if (key == "server")
			result->push_back(parseServer(&curr));
		else
			exit(printError());
	}
	std::cout << "> Parsing finished ..." << std::endl;
	return (result);
}

Location	Parser::parseLocation(std::size_t *i)
{}

Server	Parser::parseServer(std::size_t *i)
{
	Server  result;
	std::string sep = " \n\t";
	std::size_t start;
	std::size_t end;
	std::size_t prev = _content.find_first_not_of(sep, *i);

	if (prev == std::string::npos || _content[prev] != '{')
		exit(printError());
	prev++;
	std::size_t curr = _content.find_first_not_of(sep, prev);
	while (curr != std::string::npos)
	{
		if ((prev = _content.find_first_not_of(sep, curr)) == std::string::npos)
			exit(printError());
		start = prev;
		if ((curr = _content.find_first_of(sep, prev)) == std::string::npos)
			exit(printError());
		std::string key = _content.substr(start, curr - start);
		if (key == "}")
		{
			*i = _content.find_first_not_of(sep, curr + 1);
			break ;
		}

		if (key == "location")
			result.locations.push_back(parseLocation(&curr));
		else
		{
			if ((prev = _content.find_first_not_of(sep, curr)) == std::string::npos)
				exit(printError());
			if ((curr = _content.find_first_of('\n', prev)) == std::string::npos)
				exit(printError());
			if ((end = checkSyntax(_content.substr(start, curr - start))) == FAILED)
				exit(printError());
			if (static_cast<int>(end) == EMPTY)
				continue ;
			std::string val = _content.substr(prev, end - prev + start + 1);
			if (setServer(result, key, val) == FAILED)
				exit(printError());
		}
	}
	return (result);
}

int Parser::setLocation(Location &loc, std::string &key, std::string &val)
{}

int	Parser::setServer(Server &serv, std::string &key, std::string &val)
{}

int	Parser::checkSyntax(std::string line)
{
	std::size_t split = line.find_first_of("#");
	std::size_t find;

	if (split != std::string::npos)
	{
		line.erase(split);
		if (find = line.find_first_not_of(" \t\n") != std::string::npos)
			return (EMPTY);
	}

	std::size_t	semicol = line.find_first_of(";");
	std::size_t	find;

	if (semicol == std::string::npos)
		return (FAILED);
	if ((find = line.find_first_not_of(" \t\n", semicol + 1, line.length() \
	- semicol - 1)) != std::string::npos)
		return (FAILED);
	return (line.find_last_not_of(" \t", semicol - 1));
}

int	Parser::printError(void) const
{
	std::cout << "[ERROR] config parsing failed.\n";
}
