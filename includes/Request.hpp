/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Utils.hpp"

# define INCOMPLETE	0
# define COMPLETE	1
# define INVALID	2

class Request
{
	public:
		/* member type */
		//typedef std::vector<void (Request::*)()> listFuncForParse;
		typedef void (*FuncForParse)();
		typedef std::vector<FuncForParse>	listFuncForParse;
		Request(void);
		Request(/* args */);
		~Request();

		StringMap	_header;
		std::size_t	_contentLength;
		//std::string	_method;
		std::string	_body;

		/* Init */
		void	initVariables();
		void	initFuncForParse();
		
		/*Parse*/
		void	parseMethod();
		int		parse();
		void	FuncForParse();
		void	parsePath();

		/*Util*/
		bool	isHttpMethod(const std::string& str) const;

		/* Getter */
		std::string		GetHeader(const std::string& headerName);
		size_t			getNextWord(std::string& word, const std::string& delimiter);
		std::string		getNextWord(size_t sizeWord);

		class InvalidMethodException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Invalid Method");
				}
		};

	private:
		MethodType	_method;
		listFuncForParse	_funcforparse;
		std::string			_statusCode;
		std::string			_path;
		std::string			_request;
		bool				_headerParsed;
		size_t				_size;
		std::string			_protocolHTTP;
		std::string			_host;
		int					_port;
		size_t				_payloadSize;
		std::map<MethodType, std::string> _methods;
};

#endif