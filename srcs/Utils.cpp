/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/22 16:06:54 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/22 16:06:56 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"

StrVector split(std::string input, char delimiter)
{
	StrVector answer;
	std::stringstream ss(input);
	std::string temp;
 
	while (getline(ss, temp, delimiter))
		answer.push_back(temp);
 
	return answer;
}

MethodType	strToMethodType(std::string str)
{
	if (str == "GET")
		return (GET);
	else if (str == "POST")
		return (POST);
	else if (str == "DELETE")
		return (DELETE);
	else
		return (UNKNOWN);
}

std::string	methodTypeToStr(MethodType method)
{
	if (method == GET)
		return ("GET");
	else if (method == POST)
		return ("POST");
	else if (method == DELETE)
		return ("DELETE");
	else
		return ("UNKNOWN");
}

std::string	trimSpacesStr(std::string *str)
{
	const char* whitespaceChars = " \t"; // Space and tab
	str->erase(0, str->find_first_not_of(whitespaceChars));
	str->erase(str->find_last_not_of(whitespaceChars) + 1);
	return (*str);
}

void toLower(std::string *str)
{
	std::string::iterator ite;
	for (ite = str->begin(); ite != str->end(); ite++)
		*ite = std::tolower(*ite);
}

std::string readFd(int fd)
{
	//std::string     str;
	char            buf[BUFSIZE + 1];
	ssize_t         count;

	if (fd < 0)
		return ("");
	count = read(fd, buf, BUFSIZE);
	if (count < 0)
		return ("");
	buf[count] = '\0';
	std::string str(buf);
	return (str);
}
