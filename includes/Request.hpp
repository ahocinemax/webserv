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

# include "../includes/Utils.hpp"
# include <stdexcept>
# include <climits>

# define INCOMPLETE	0
# define COMPLETE	1
# define INVALID	2

class Request
{
	public:
		/* member type */
		typedef void (Request::*FuncForParse)();
		typedef std::vector<FuncForParse>	listFuncForParse;
		//typedef void (*FuncForParse)();
		Request(const std::string &request);
		~Request();

		StringMap	_header;
		std::size_t	_contentLength;
		//std::string	_method;

		/* Init */
		void	initVariables();
		void	initFuncForParse();
		
		/*Parse*/
		void	parseMethod();
		int		parse();
		void	FuncForParseHeader();
		void	parsePath();
		void	parseHttpProtocol();
		void	parseHeaders();
		bool	ConversionPort(const std::string & str, int *num);
		bool	parseHeaderHost();
		void	checkHeaders();
		int		checkChunk();
		void	parseBody();
		

		/*Util*/
		bool	isHttpMethod(const std::string& str) const;
		bool	isHeader(const std::string& headerName);
		void	ContentLength();

		/* Getter */
		std::string		getHeader(const std::string& headerName);
		size_t			getNextWord(std::string& word, const std::string& delimiter);
		std::string		getNextWord(size_t sizeWord);
		int				getStatusCode() const;
		std::string		getMethod() const;

		std::string		getPath() const;
		std::string		getBody() const;
		std::string		getQuery() const;
		std::string		getProtocolHTTP() const;
		size_t			getSize() const;
		std::string		getHost() const;
		int				getPort() const;

		/*test*/
		void			PrintHeader();

		class InvalidMethodException : public std::exception
		{
			public:
				virtual const char* what() const throw()
				{
					return ("Invalid Method");
				}
		};
		class Error : public virtual std::exception {
		public:
			Error(const char* msg) : _msg(msg) {}

			const char* what() const throw() {
				return _msg;
			}

		private:
			const char* _msg;
		};

	private:
		std::string			_method;
		listFuncForParse	_funcforparse;
		int					_statusCode;
		int					_requestStatus;
		std::string			_path;
		std::string			_query;
		std::string			_request;
		std::string			_body;
		bool				_headerParsed;
		bool				_chunked;
		size_t				_size;
		std::string			_protocolHTTP;
		std::string			_host;
		int					_port;
		int					_payloadSize;
		std::map<MethodType, std::string> _methods;
};

#endif