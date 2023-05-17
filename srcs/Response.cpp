#include "Response.hpp"

Response::Response(void) {}

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