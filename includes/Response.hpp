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
# define RESPONSE_HPP
# include "Request.hpp"
# include "Utils.hpp"

class Response
{
	public:
		Response(void);
		Response(std::string status);
		~Response();

		std::string	makeHeader(bool appendBody = false);
		void		setDefaultErrorMessage(void);

		std::string	getStatusCode(void) const;
		std::string	getStatusMessage(void) const;
		void		setCustomizeErrorMessage(std::string &message);
		void		addHeader(std::string key, std::string value);
		std::string	getBody(void) const { return (_message); }
		Request*	getRequest();

	private:
		std::string	_statusCode;
		std::string	_statusMessage;
		std::string	_message;
		StringMap	_head;
		Request		*_request;

		std::string	getDate(void);
};

#endif