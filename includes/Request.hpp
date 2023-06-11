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
		typedef void (Request::*FuncForParse)();
		typedef std::vector<FuncForParse>	listFuncForParse;
		Request(const std::string &request);
		~Request();

		int				_requestStatus;
		int				_statusCode;
		StringMap		_header;
		
		/* Init */
		void	initVariables();
		void	initFuncForParse();
		
		/*Parse*/
		void	parseMethod();
		void	parse();
		int		FuncForParseHeader();
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
		void	appendCgiBody(const std::string &output);

		/* Getter */
		std::string		getHeader(const std::string& headerName);
		size_t			getNextWord(std::string& word, const std::string& delimiter);
		std::string		getNextWord(size_t sizeWord);
		std::string		getMethod() const;

		std::string		getPath() const;
		std::string		getRoot() const;
		std::string		getBody() const;
		size_t			getPayloadSize() const;
		std::string		getQuery() const;
		std::string		getProtocolHTTP() const;
		size_t			getSize() const;
		std::string		getHost() const;
		int				getPort() const;
		void			setRoot(std::string& root);
		void			setCgiBody(std::string& body);
		std::string		getCgiBody(int index);
		StrVector		getCgiBody() const;

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
		std::string							_method;
		listFuncForParse					_funcforparse;
		std::string							_path;
		std::string							_query;
		std::string							_request;
		std::string							_body;
		std::vector<std::string>			_cgibody;
		bool								_headerParsed;
		bool								_chunked;
		size_t								_size;
		std::string							_protocolHTTP;
		std::string							_host;
		int									_port;
		size_t								_payloadsize;
		std::string							_root;
		std::map<MethodType, std::string>	_methods;
};

#endif