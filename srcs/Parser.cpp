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
#include "Utils.hpp"

Parser::Parser(const char *config_file)
{
	std::string     line;
	std::ifstream   fs;

	line.clear();
	fs.open(config_file, std::ifstream::in);
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
		std::cerr << "Error: Cannot open file " << config_file << std::endl;
		exit(1);
	}
}

Parser::~Parser(void) {}

std::vector<Server>	*Parser::parse(void)
{
	std::vector<Server> *result = new std::vector<Server>();

	std::cout << "> Parsing config file ..." << std::endl;
	std::size_t prev = 0;
	std::size_t curr = _content.find_first_not_of(SEP, prev);

	if (curr == std::string::npos)
		exit(printError(1));
	while (curr != std::string::npos)
	{
		prev = _content.find_first_not_of(SEP, curr);
		curr = _content.find_first_of(SEP, prev);
		std::string key = _content.substr(prev, curr - prev);
		if (key == "server")
			result->push_back(parseServer(&curr));
		else
			exit(printError(2));
	}
	std::cout << "> Parsing finished ..." << std::endl;
	return (result);
}

Location	Parser::parseLocation(std::size_t *i)
{
	Location  result;
	std::size_t start;
	std::size_t end;
	std::size_t prev = _content.find_first_not_of(SEP, *i);
	std::size_t curr = _content.find_first_of("\n{", prev);
	result._path = _content.substr(prev, curr - prev);
	prev = _content.find_first_not_of(SEP, curr);
	if (prev == std::string::npos || _content[prev] != '{')
		exit(printError(3));
	prev++;
	curr = _content.find_first_of(SEP, prev);
	while (curr != std::string::npos)
	{
		if ((prev = _content.find_first_not_of(SEP, curr)) == std::string::npos)
			exit(printError(4));
		start = prev;
		if ((curr = _content.find_first_of(SEP, prev)) == std::string::npos)
			exit(printError(5));
		std::string key = _content.substr(start, curr - start);
		if (key == "}")
		{
			*i = _content.find_first_not_of(SEP, curr + 1);
			break ;
		}
		else
		{
			if ((prev = _content.find_first_not_of(SEP, curr)) == std::string::npos)
				exit(printError(6));
			if ((curr = _content.find_first_of('\n', prev)) == std::string::npos)
				exit(printError(7));
			if ((end = checkSyntax(_content.substr(start, curr - start))) == FAILED)
				exit(printError(8));
			if (static_cast<int>(end) == EMPTY)
				continue ;
			std::string val = _content.substr(prev, start + 1 + end - prev);
			if (setLocation(&result, key, val) == FAILED)
				exit(printError(9));
		}
	}
	return (result);
}

Server	Parser::parseServer(std::size_t *i)
{
	Server  result;
	std::size_t start;
	std::size_t end;
	std::size_t prev = _content.find_first_not_of(SEP, *i);

	if (prev == std::string::npos || _content[prev] != '{')
		exit(printError(10));
	prev++;
	std::size_t curr = _content.find_first_not_of(SEP, prev);
	while (curr != std::string::npos)
	{
		if ((prev = _content.find_first_not_of(SEP, curr)) == std::string::npos)
			exit(printError(11));
		start = prev;
		if ((curr = _content.find_first_of(SEP, prev)) == std::string::npos)
			exit(printError(12));
		std::string key = _content.substr(start, curr - start);
		if (key == "}")
		{
			*i = _content.find_first_not_of(SEP, curr + 1);
			break ;
		}

		if (key == "location")
			result.locations.push_back(parseLocation(&curr));
		else
		{
			if ((prev = _content.find_first_not_of(SEP, curr)) == std::string::npos)
				exit(printError(13));
			if ((curr = _content.find_first_of('\n', prev)) == std::string::npos)
				exit(printError(14));
			if ((end = checkSyntax(_content.substr(start, curr - start))) == FAILED)
				exit(printError(15));
			if (static_cast<int>(end) == EMPTY)
				continue ;
			std::string val = _content.substr(prev, end - prev + start + 1);
			if (setServer(&result, key, val) == FAILED)
				exit(printError(16));
		}
	}
	return (result);
}

int Parser::setLocation(Location *loc, std::string &key, std::string &val)
{
	std::size_t value;
	// std::cout << "key: \"" << key << "\" val: {" << val << "}" << std::endl;
	if (key == "root")
		loc->_root = val;
	else if (key == "client_body_limit")
		loc->_client_body_limit = atoi(val.c_str());
	else if (key == "index")
	{
		std::vector<std::string> tmp = split(val, ' ');
		for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); it++)
			loc->_index.push_back(*it);
	}
	else if (key == "allow_methods")
	{
		std::vector<std::string> tmp = split(val, ' ');
		for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); it++)
			loc->_allowMethods.push_back(loc->methodType(*it));
	}
	else if (key == "cgi_info")
	{
		std::size_t	key = val.find_first_of(" \t");
		if (key == std::string::npos)
			return FAILED;
		value = val.length();
		if (value == key)
			return EMPTY;
		loc->_cgi[val.substr(0, key)] = val.substr(key + 1, value);
	}
	else
	{
		std::cerr << RED "Error:" RESET " Invalid key in location part" RED " {" << key << ": " << val << "}" RESET << std::endl;
		std::cerr << "This parameter will be ignored." RESET << std::endl;
		return (SUCCESS);
	}
	std::cout << YELLOW "added " << GREEN << key << ": " << BLUE << \
	val << YELLOW " to location" RESET << std::endl;
	return SUCCESS;
}

int	Parser::setServer(Server *serv, std::string &key, std::string &val)
{
	if (key == "server_name")
		serv->server_name = val;
	else if (key == "listen")
	{
		if (val.find_first_of(':') == std::string::npos)
		{
			serv->_ipAddress = "0.0.0.0";
			serv->_port = atoi(val.c_str());
		}
		else
		{
			std::size_t split = val.find_first_of(':');
			if (split == std::string::npos)
				return (FAILED);
			serv->_ipAddress = val.substr(0, split);
			serv->_port = val.substr(split + 1, val.length() - 1);
		}
	}
	else if (key == "root")
		serv->root = val;
	else if (key == "index")
	{
		std::vector<std::string> tmp = split(val, ' ');
		for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); it++)
			serv->index.push_back(*it);
	}
	else if (key == "autoindex")
		serv->autoindex = (val == "on") ? true : false;
	else if (key == "client_body_limit")
		serv->client_body_limit = atoi(val.c_str()); // check before atoi
	else if (key == "error_page")
	{
		std::vector<std::string> tmp = split(val, ' ');
		std::string tmp2 = tmp[tmp.size() - 1];
		for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end() - 1; it++)
		{
			int status = atoi((*it).c_str());
			if (serv->error_pages.find(status) != serv->error_pages.end())
				continue;
			serv->error_pages[status] = tmp2;
		}
	}
	else if (key == "return")
	{
		std::vector<std::string> tmp = split(val, ' ');
		serv->redirect_status = atoi(tmp[0].c_str());
		serv->redirect_url = tmp[1];
	}
	else if (key == "recv_timeout")
		serv->recv_timeout.tv_sec = atoi(val.c_str());
	else if (key == "send_timeout")
		serv->send_timeout.tv_sec = atoi(val.c_str());
	else if (key == "allow_methods")
	{
		std::vector<std::string> tmp = split(val, ' ');
		for (std::vector<std::string>::iterator it = tmp.begin(); it != tmp.end(); it++)
			serv->allowMethods.push_back(serv->methodType(*it));
	}
	else
	{
		std::cerr << RED "Error:" RESET " Invalid key in server part" RED " {" << key << ": " << val << "}" RESET << std::endl;
		std::cerr << "This parameter will be ignored." RESET << std::endl;
		return (SUCCESS);
	}
	std::cout << YELLOW "added " << GREEN << key << ": " << BLUE << val \
	<< YELLOW " to server" RESET << std::endl;
	return (SUCCESS);
}

int	Parser::checkSyntax(std::string line)
{
	std::size_t split = line.find_first_of("#");
	std::size_t find;

	if (split != std::string::npos)
	{
		line.erase(split);
		if ((find = line.find_first_not_of(SEP)) != std::string::npos)
			return (EMPTY);
	}

	std::size_t	semicol = line.find_first_of(";");

	if (semicol == std::string::npos)
		return (FAILED);
	if ((find = line.find_first_not_of(SEP, semicol + 1, line.length() \
	- semicol - 1)) != std::string::npos)
		return (FAILED);
	return (line.find_last_not_of(" \t", semicol - 1));
}

int	Parser::printError(int error_code) const
{
	std::cerr << "[ERROR] config parsing failed." << error_code << std::endl;
	return (FAILED);
}
