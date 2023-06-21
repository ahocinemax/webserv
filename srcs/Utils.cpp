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


std::string	generateCopyFile(const std::string& dir, const std::string& file)
{
	const size_t			pos_extention = file.rfind('.');
	const std::string		extension = file.substr(pos_extention);
	std::string				filename = file.substr(0, pos_extention);
	int						last_copy = 1;

	if (filename.empty())
		filename = extension;
	if (*(dir.rbegin()) != '/' && *(filename.begin()) != '/')
		filename.insert(0, "/");

	std::string				copy_filepath = dir + filename + "_copy" + extension;
	while (AccessiblePath(copy_filepath))
	{
		const std::string	num = to_string(++last_copy);
		copy_filepath = dir + filename + "_copy" + num + extension;
	}
	if (!copy_filepath.empty() && copy_filepath[0] == '.')
        copy_filepath.erase(0, 1);
	return (copy_filepath);
}

bool	AccessiblePath(const std::string& path)
{
	int	ret;

	ret = access(path.c_str(), F_OK); 
	return (!ret);
}

void	SigPipeIgnore(int sig) { (void) sig; }

void	SigpipeSet(int state)
{
	void	(*oldHandler_SigPipe)(int) = 0;

	if (state == 0) {
		oldHandler_SigPipe = signal(SIGPIPE, SigPipeIgnore);
	} else if (state == 1) {
		signal(SIGPIPE, oldHandler_SigPipe);
	}
}

std::string decodeURIComponent(std::string encoded)
{
    std::string decoded = encoded;
    std::string haystack;

    int dynamicLength = decoded.size() - 2;

    if (decoded.size() < 3)
        return decoded;

    regex_t regex;
    regmatch_t match;

    if (regcomp(&regex, "%[0-9A-F]{2}", REG_EXTENDED) != 0)
        return decoded;

    for (int i = 0; i < dynamicLength; i++)
    {
        haystack = decoded.substr(i, 3);

        if (regexec(&regex, haystack.c_str(), 1, &match, 0) == 0)
        {
            haystack.replace(0, 1, "0x");
            unsigned int rc = 0;
            std::istringstream(haystack) >> std::hex >> rc;
            decoded.replace(i, 3, 1, static_cast<char>(rc));
			i++;
        }

        dynamicLength = decoded.size() - 2;
    }

    regfree(&regex);

    return decoded;
}

std::string encodeURIComponent(std::string decoded)
{
    std::ostringstream oss;
    std::string pattern("[!'\\(\\)*-.0-9A-Za-z_~]");

    regex_t regex;
	regmatch_t match;

    if (regcomp(&regex, pattern.c_str(), REG_EXTENDED) != 0)
        return decoded;

    for (size_t i = 0; i < decoded.size(); i++)
    {
        std::string c(1, decoded[i]);

        if (regexec(&regex, c.c_str(), 1, &match, 0) == 0)
        {
            oss << c;
        }
        else
        {
            oss << "%" << std::uppercase << std::hex << (unsigned int)(0xFF & decoded[i]);
        }
    }

    regfree(&regex);

    return oss.str();
}