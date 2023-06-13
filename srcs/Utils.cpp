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
#include <iomanip>

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
	char buffer[BUFFER_SIZE + 1];
	int totalBytes = 0;
	int bytesRead = 0;
	std::string str;
	if (fd < 0)
		return ("");
	while ((bytesRead = read(fd, buffer, BUFFER_SIZE)) > 0)
	{
		str.append(buffer, bytesRead);
		totalBytes += bytesRead;
	}
	if (totalBytes <= 0)
	{
		return ("");
	}

	std::cout << RED "readFd: \n" RESET << str << std::endl;
	return (str);
}

std::string convertToOctets(int octets)
{
	std::ostringstream result;

	if (octets < 1024)
		result << octets << " octets";
	else if (octets < 1024 * 1024)
	{
		double kilo = static_cast<double>(octets) / 1024;
		result << std::fixed << std::setprecision(1) << kilo << " Kb";
	}
	else if (octets < 1024 * 1024 * 1024)
	{
		double mega = static_cast<double>(octets) / (1024 * 1024);
		result << std::fixed << std::setprecision(1) << mega << " Mb";
	}
	else
	{
		double giga = static_cast<double>(octets) / (1024 * 1024 * 1024);
		result << std::fixed << std::setprecision(1) << giga << " Gb";
	}

	return (result.str());
}
bool	convertHttpCode(const std::string& str, int* code)
{
	size_t	found;

	found = str.find_first_not_of("0123456789");
	if (found != std::string::npos)
		return (false);
	*code = atoi(str.c_str());
	return (*code >= 0 && *code <= 505);
}
