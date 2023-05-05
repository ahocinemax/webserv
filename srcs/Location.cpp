#include "Location.hpp"

Location::Location(/* args */) : _client_body_limit(0), _path(""), _root("")
{
}

Location::~Location(void)
{
}

void	Location::printLocation(void) const
{
	std::cout << "-----------------Location Info: -----------------" << std::endl;
	std::cout << "> path: " << this->_path << std::endl;
	std::cout << "> root: " << this->_root << std::endl;
	std::cout << "> client_body_limit: " << this->_client_body_limit << std::endl;
	std::cout << "> allow_methods: " << std::endl;
	for (unsigned long i = 0; i < _allowMethods.size(); i++)
	{
		switch (_allowMethods[i])
		{
			case GET:
				std::cout << "\tGET" << std::endl;
				break;
			case POST:
				std::cout << "\tPOST" << std::endl;
				break;
			case PUT:
				std::cout << "\tPUT" << std::endl;
				break;
			case DELETE:
				std::cout << "\tDELETE" << std::endl;
				break;
			case HEAD:
				std::cout << "\tHEAD" << std::endl;
				break;
			case OPTIONS:
				std::cout << "\tOPTIONS" << std::endl;
				break;
			case TRACE:
				std::cout << "\tTRACE" << std::endl;
				break;
			case CONNECT:
				std::cout << "\tCONNECT" << std::endl;
				break;
			default:
				break;
		}
	}
	std::cout << "> index: " << std::endl;
	for (size_t i = 0; i < this->_index.size(); i++)
		std::cout << "\t" << this->_index[i] << std::endl;
	std::cout << "> cgi_info: " << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = this->_cgi.begin();
	it != this->_cgi.end(); it++)
		std::cout << "\t" << it->first << ", " << it->second << std::endl;
}

MethodType	Location::methodType(std::string str)
{
	if (str == "GET")
		return (GET);
	else if (str == "HEAD")
		return (HEAD);
	else if (str == "POST")
		return (POST);
	else if (str == "PUT")
		return (PUT);
	else if (str == "DELETE")
		return (DELETE);
	else if (str == "CONNECT")
		return (CONNECT);
	else if (str == "OPTIONS")
		return (OPTIONS);
	else if (str == "TRACE")
		return (TRACE);
	else
		return (UNKNOWN);
}

std::string	Location::getRoot(void) const
{
	return (this->_root);
}

std::string	Location::getPath(void) const
{
	return (this->_path);
}

std::string	Location::getCgi(std::string extension) const
{
	for (std::map<std::string, std::string>::const_iterator it = this->_cgi.begin();
	it != this->_cgi.end(); it++)
	{
		if (it->first == "." + extension)
			return (it->second);
	}
	return ("");
}