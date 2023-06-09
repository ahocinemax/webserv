/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/22 16:06:54 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/22 16:06:56 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(void) : _statusCode(""), _cgibody("") {}

Response::Response(std::string status)
{
	if (status.length() < 3)
	{
		_statusCode = "404";
		_statusMessage = "Not Found";
		std::cerr << RED "Error:" RESET " failed to send file" << std::endl;
	}
	else
	{
		_statusCode		= status.substr(0, 3);
		_statusMessage	= status.substr(4);
	}
}

Response::~Response() {}

std::string	Response::makeHeader(bool appendBody)
{
	std::string result = "HTTP/1.1 " + getStatusCode() + " " + getStatusMessage() + CRLF;
	for (StringMap::iterator it = _head.begin() ; it != _head.end() ; it++)
		result.append(it->first + ": " + it->second + CRLF);
	result += CRLF;
	if (appendBody)
		result += _message;
	return (result);
}

void	Response::setDefaultStatusPage() // page d'erreur par défaut
{
	std::cout << "> sending default error message" << std::endl;
	_message.clear();
	_message = "<!DOCTYPE html> \
	<html> \
		<head> \
			<title>Webserv - Page d'erreur (" + getStatusCode() + ") </title> \
			<style> \
				body { font-family: Arial, sans-serif; background-color: #f2f2f2; color: #333333; margin: 0; padding: 0; } \
				.error-container { text-align: center; margin-top: 200px; } \
				h1 { font-size: 36px; color: #e74c3c; } \
				p { font-size: 18px; } \
			</style> \
		</head> \
		<body> \
			<div class='error-container'> \
				<h1>Erreur " + getStatusCode() +"</h1> \
				<p>" + getStatusMessage() + ".</p> \
			</div> \
		</body> \
	</html>";
}

std::string	Response::getStatusCode(void) const { return (_statusCode); }

std::string	Response::getStatusMessage(void) const { return (_statusMessage); }

void	Response::setCustomizeStatusPage(std::string &message)
{ _message = message; }

void	Response::addHeader(std::string key, std::string value)	
{ _head.insert(std::make_pair(key, value)); }

Request*	Response::getRequest(){return (_request);}

void	Response::setCgiBody(const std::string& body)
{ 
	_cgibody = body; 
}

std::string	Response::getHeader(std::string key)
{
	return (_head.find(key) != _head.end() ? _head[key] : "");
}

void Response::setStatusCode(int status)
{
	_statusCode = status;
}

void Response::parseCgiStatusLine()
{
	std::string name;
	std::string value;
	int code;

	name = _cgibody.substr(0, _cgibody.find(":"));
	if (name != "Status" && name != "status")
		return;
	getNextWord(_cgibody, name, ":");
	_cgibody.erase(0, _cgibody.find_first_not_of(" "));
	getNextWord(_cgibody, value, " ");
	trimSpacesStr(&value);
	if (convertHttpCode(value, &code))
	{
		getNextWord(_cgibody, value, "\r\n");
		trimSpacesStr(&value);
		if (_statusCode == value)
			setStatusCode(code);
	}
}

void Response::parseCgiBody()
{
	size_t pos;
	std::string headerName;
	std::string headerValue;

	/*if _cgibody doesn't have header*/
	if (_cgibody.find("\r\n\r\n") == std::string::npos && _cgibody.find("\n\n") == std::string::npos)
		return;
	pos = 0;
	parseCgiStatusLine();
	while (pos != std::string::npos && _cgibody.find("\r\n"))
	{
		pos = getNextWord(_cgibody, headerName, ":");
		if (pos == std::string::npos)
			break;
		getNextWord(_cgibody, headerValue, "\r\n");
		trimSpacesStr(&headerValue);
		addHeader(headerName, headerValue);
	}
	getNextWord(_cgibody, headerName, "\r\n");
}

size_t Response::getNextWord(std::string &body, std::string &word, std::string const &delimiter)
{
	size_t pos;
	size_t totalSize;

	pos = body.find(delimiter);
	std::string nextWord(body, 0, pos);
	totalSize = pos + delimiter.length();
	body.erase(0, totalSize);
	word = nextWord;
	return (pos);
}

std::string	Response::getDate(void) const
{
	time_t		now = time(0);
	struct tm	tstruct;
	char		buf[80];

	tstruct = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %G %X GMT", &tstruct);
	std::string date(buf);
	return (date);
}
