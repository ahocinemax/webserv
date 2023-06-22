/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                         :+:      :+:    :+:   */
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
		Request &operator=(const Request &other);
		~Request();
		Request() {}
		int				_requestStatus;
		int				_statusCode;
		StringMap		_header;
		
		/* Init */
		void			initVariables();
		void			initFuncForParse();
		
		/*Parse*/
		void			parseMethod();
		void			parse();
		void			parseHeaders();
		void			parseBody();
		int				FuncForParseHeader();
		void			parsePath();
		void			parseHttpProtocol();
		bool			parseHeaderHost();
		void			checkHeaders();
		int				checkChunk();
		

		/*Util*/
		bool			isHeader(const std::string& headerName);
		void			ContentLength();
		void			appendCgiBody(const std::string &output);
		bool			isHttpMethod(const std::string& str) const;
		void			insertUploadpath(size_t pos, const std::string& uploadpath);

		/* Get */
		size_t			getNextWord(std::string& word, const std::string& delimiter);
		std::string		getNextWord(size_t sizeWord);
		std::string		getHeader(const std::string& headerName);
		std::string		getMethod() const;
		std::string		getPath() const;
		std::string		getRoot() const;
		std::string		getBody() const;
		std::string		getCgiBody(int index);
		StrVector		getCgiBody() const;
		std::string		getQuery() const;
		std::string		getProtocolHTTP() const;
		size_t			getSize() const;
		size_t			getPayloadSize() const;
		std::string		getHost() const;
		int				getPort() const;

		/* Set */
		void			setRoot(std::string& root);
		void			setCgiBody(std::string& body);
		void			setPath(std::string& path);

		/*test*/
		void			PrintHeader();

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