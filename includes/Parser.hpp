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

class Parser
{
	public:
		Parser(std::string config_file);
		~Parser();

		void	parse();

	private:
		std::string					_server_name;
		std::string					_root;
		std::vector<std::string>	_index;
		std::map<int, std::string>	_error_page;
		int							_client_max_body_size;
		bool						_autoindex;
		std::string					_ip_address;
		std::vector<MethodTypes>	_allowed_methods;
		Location					_location;
};

#endif
