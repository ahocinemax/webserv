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

Response::Response(void) : _statusCode(0) {}

Response::Response(std::string statusCode) : _statusCode(statusCode) {}

Response::~Response() {}

std::string	Response::makeHeader(bool appendBody)
{
	std::string result = "HTTP/1.1 " + getStatusCode() + " " + getStatusMessage() + "\r\n";
	for (StringMap::iterator it = _head.begin() ; it != _head.end() ; it++)
		result.append(it->first + ": " + it->second + "\r\n");
	result += "\r\n";
	if (appendBody)
		result += _message;
	return (result);
}

std::string	Response::getStatusCode(void) const { return (_statusCode); }

std::string	Response::getStatusMessage(void) const { return (_statusMessage); }

void	Response::setMessage(std::string &message) { _message = message; }

void	Response::makeMessage()
{
	_message.clear();
	_message = "<!DOCTYPE html> \
	<html> \
	<head> \
	<title>Page d'erreur (" + getStatusCode() + ") \
	</title> \
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