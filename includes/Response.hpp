#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Utils.hpp"

class Response
{
	public:
		Response(void);
		Response(std::string status);
		~Response();

		std::string	makeHeader(bool appendBody = false);

		std::string	getStatusCode(void) const;
		std::string	getStatusMessage(void) const;

	private:
		std::string	_statusCode;
		std::string	_statusMessage;
		std::string	_message;
		StringMap	_head;

		std::string	getDate(void);
};

#endif