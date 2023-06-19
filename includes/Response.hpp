/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include "Request.hpp"
#include "Utils.hpp"

class Response
{
	public:
		Response(std::string status);
		Response(void);
		~Response();

		std::string	makeHeader(bool appendBody = false);
		void		setDefaultStatusPage(void);

	std::string getDate(void) const;
	std::string getStatusCode(void) const;
	std::string getStatusMessage(void) const;
	void 		setCustomizeStatusPage(std::string &message);
	void 		addHeader(std::string key, std::string value);
	std::string getBody(void) const { return (_message); }
	void		setBody(std::string message) { _message = message; };
	std::string getCgiBody(int index) const { return (_cgibody[index]); }
	Request 	*getRequest();
	std::string	getHeader(std::string key);

	void 		setCgiBody(const std::string& cgibody);
	void		setStatusCode(int status);
	void		parseCgiStatusLine(std::string &body);
	void		parseCgiBody(std::string body);
	size_t		getNextWord(std::string& body, std::string &word, std::string const& delimiter);
	std::string					_message;

private:
	std::string					_statusCode;
	std::string					_statusMessage;
	StringMap					_head;
	std::vector<std::string>	_cgibody;
	Request*					_request;
};

#endif