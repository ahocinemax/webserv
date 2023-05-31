/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Utils.hpp"

class Location
{
	public:
		Location(/* args */);
		~Location();
		void				printLocation(void) const;
		std::string			getRoot(void) const;
		std::string			getPath(void) const;
		std::string			getCgi(std::string extension) const;

	public:
		int						_client_body_limit;
		MethodVector			_allowMethods;
		StrVector				_index;
		std::string				_path;
		std::string				_root;
		StringMap				_cgi;
};

#endif
