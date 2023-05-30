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

Response::Response(void) : _statusCode("") {}

Response::Response(std::string status) : _statusCode(status.substr(0, 3)),
										 _statusMessage(status.substr(4)) {}

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

void	Response::setDefaultErrorMessage() // page d'erreur par défaut
{
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

void	Response::setCustomizeErrorMessage(std::string &message) { _message = message; }

void	Response::addHeader(std::string key, std::string value) { _head[key] = value; } // Utiliser insert(make_pair(key, value)) ?

Request*	Response::getRequest(){return (_request);}