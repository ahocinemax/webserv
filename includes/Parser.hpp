/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/07 13:25:29 by ahocine           #+#    #+#             */
/*   Updated: 2023/03/07 05:54:51 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include "webserv.hpp"
# include "Server.hpp"
# include <fstream>

class Parser
{
	public:
		Parser(const char *config_file);
		~Parser(void);

		std::vector<Server>	*parse(void);

	private:
		std::string	_content;

		Location	parseLocation(std::size_t *i);
		Server		parseServer(std::size_t *i);

		int			setLocation(Location *loc, std::string &key, std::string &val);
		int			setServer(Server *serv, std::string &key, std::string &val);
		
		int			checkSyntax(std::string line);
		int			printError(void) const;
};

#endif
