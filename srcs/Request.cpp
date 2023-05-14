#include "Request.hpp"


std::string		Request::GetHeader(const std::string& headerName)
{
	if (_header.find(headerName) != _header.end())
		return (_header[headerName]);
	return ("");
}