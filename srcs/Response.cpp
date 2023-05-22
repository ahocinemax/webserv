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

std::string    Response::makeHeader(bool appendBody)
{
    std::string header = "HTTP/1.1 " + getStatusCode() + " " + getStatusMessage() + "\r\n";
    for (StringMap::iterator it = _head.begin() ; it != _head.end() ; it++)
        header.append(it->first + ": " + it->second + "\r\n");
    header += "\r\n";
    if (appendBody)
        header += _message;
    return (header);
}

std::string    Response::getStatusCode(void) const { return (_statusCode); }

std::string    Response::getStatusMessage(void) const { return (_statusMessage); }