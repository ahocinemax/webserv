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
		Response(std::string status);
		Response(void);
		~Response();

		std::string	makeHeader(bool appendBody = false);
		void		setDefaultStatusPage(void);

		std::string	getDate(void) const;
		std::string	getStatusCode(void) const;
		std::string	getStatusMessage(void) const;
		void		setCustomizeStatusPage(std::string &message);
		void		addHeader(std::string key, std::string value);
		std::string	getBody(void) const { return (_message); }
		Request*	getRequest();

	private:
		std::string	_statusCode;
		std::string	_statusMessage;
		std::string	_message;
		StringMap	_head;
		Request		*_request;
};

#endif