#include "Request.hpp"


/*
	sample:
	Host: localhost:8080
	Connection: keep-alive
	sec-ch-ua: "Google Chrome";v="113", "Chromium";v="113", "Not-A.Brand";v="24"
	sec-ch-ua-mobile: ?0
	sec-ch-ua-platform: "Linux"
	Upgrade-Insecure-Requests: 1
	User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,**;q=0.8,application/signed-exchange;v=b3;q=0.7
	Sec-Fetch-Site: none
	Sec-Fetch-Mode: navigate
	Sec-Fetch-User: ?1
	Sec-Fetch-Dest: document
	Accept-Encoding: gzip, deflate, br
	Accept-Language: en-US,en;q=0.9,ja;q=0.8,fr;q=0.7
	Cookie: sessionId=yekwp6nv9y1po66y
	-----
	sample : body with chunked
	HTTP/1.1 200 OK
	Content-Type: text/plain
	Transfer-Encoding: chunked

	7\r\n
	Hello, \r\n
	9\r\n
	world!\r\n
	0\r\n
	\r\n

*/

Request::Request(const std::string& request) : _request(request)
{
	initVariables();
	initFuncForParse();
}

Request::~Request() {}

void	Request::initVariables()
{
	_statusCode = OK;
	_requestStatus = INCOMPLETE;
	_method = UNKNOWN;
	_path = "";
	_query = "";
	_protocolHTTP = "";
	_size = 0;
	_body = "";
	_host = "";
	_port = 0;
	_headerParsed = false;
	_chunked = false;
	_payloadsize = 0;
	_methods.insert(std::make_pair(GET, "GET"));
	_methods.insert(std::make_pair(POST, "POST"));
	_methods.insert(std::make_pair(DELETE, "DELETE"));
	_methods.insert(std::make_pair(UNKNOWN, "UNKNOWN"));
}

void	Request::initFuncForParse()
{
    _funcforparse.insert(_funcforparse.end(), &Request::parseMethod);
    _funcforparse.insert(_funcforparse.end(), &Request::parsePath);
    _funcforparse.insert(_funcforparse.end(), &Request::parseHttpProtocol);
    _funcforparse.insert(_funcforparse.end(), &Request::parseHeaders);
    _funcforparse.insert(_funcforparse.end(), &Request::checkHeaders);
}

void Request::parseMethod()
{
	std::string method;

	getNextWord(method, " ");
	if (isHttpMethod(method) == false)
	{
		throw InvalidMethodException();
	}
	//_method = strToMethodType(method);
	_method = method;
}

void	Request::parsePath()
{
	std::string		path;
	size_t			pos;

	getNextWord(path, " ");
	if (path == "" || path[0] != '/')
	{
		_statusCode = BAD_REQUEST;
		throw Error("400 Bad Request");
	}
	if (path.length() > 2000) // ou bien 3000? max longuer de URL
	{
		_statusCode = URI_TOO_LONG;
		throw Error("414 URI Too Long");
	}
	pos = path.find("?");
	if (pos != std::string::npos)
	{
		_query = path.substr(pos + 1);
		path.erase(pos);
	}
	_path = path;
}

void	Request::parseHttpProtocol()
{
	std::string protocolHTTP;
	
	getNextWord(protocolHTTP, CRLF);
	if (protocolHTTP.find("HTTP") == std::string::npos)
	{
		_statusCode = BAD_REQUEST;
		throw Error("400 Bad Request");
	}
	if (protocolHTTP != "HTTP/1.1")
	{
		_statusCode = HTTP_VERSION_NOT_SUPPORTED;
		throw Error("505 HTTP Version Not Supported");
	}
	_protocolHTTP = protocolHTTP;
}

void	Request::parseHeaders()
{
		std::string	headerName;
		std::string headerVal;
		size_t pos;

		pos = 0;
		while (pos != std::string::npos && _request.find(CRLF))
		{
			pos = getNextWord(headerName, ":");
			if (pos == std::string::npos)
				break;
			toLower(&headerName);
			getNextWord(headerVal, CRLF);	
			trimSpacesStr(&headerVal);
			if (isHeader(headerName))
			{
				_statusCode = BAD_REQUEST;
				throw Error("400 Bad Request");	//->this header is already existed
			}
			_header[headerName] = headerVal;
			if (_payloadsize > 10000)
			{
				_statusCode = PAYLOAD_TOO_LARGE;
				throw Error("413 Payload Too Large");	//->this header is already existed
			}
		}
		getNextWord(headerName, CRLF);// separete between header/body
}

bool	Request::parseHeaderHost()
{
	StringMap::const_iterator ite;
	size_t pos;
	
	ite = _header.find("host");
	if (ite == _header.end())
		return (false);
	_host = ite->second;
	pos = _host.find(":");
	if (pos == std::string::npos)
		return (true);
	_host = _host.substr(0, pos);
	if (pos + 1 != std::string::npos)
	{
		size_t tmp;
		std::string str;
		str = ite->second.substr(pos + 1);
		tmp = str.find_first_not_of("0123456789");
		if (tmp != std::string::npos)
			return (false);
		_port = atoi(str.c_str());
		return (_port >= 1 && _port <= 65535);
	}
	else
		return (false);
}

void	Request::checkHeaders()
{
	/*
		memo:
		il faut faire:
		-> parse entre host / port (localhost:8080)
		-> check content-length
		-> check transfer-encoding (chunked)
		-> bool _headerParsed = true
	*/
	StringMap::const_iterator ite;
	if (!parseHeaderHost())
	{
		_statusCode = BAD_REQUEST;
		throw Error("400 Bad Request");
	}
	ContentLength();
	if (_method != "POST") //GET et DELETE n'ont pas besoin de body
	{
		_headerParsed = true;
		return ;
	}
	ite = _header.find("Transfer-Encoding");
	if (ite != _header.end() && ite->second.find("chunked"))
		_chunked = true;
	if (_header.find("Content-Length") == _header.end())
	{
		_statusCode = BAD_REQUEST;
		throw Error("400 Bad Request");
	}
	_headerParsed = true;
}

int	Request::checkChunk()
{
	/*
		chunked exemplle:
		7\r\n
		Hello, \r\n
		9\r\n
		world!\r\n
		0\r\n
		\r\n
	*/
	std::string	chunk;
	size_t		size;

	while (1)
	{
		size = 0;
		if (getNextWord(chunk, CRLF) == std::string::npos)
		{
			std::cerr << "chunked error" << std::endl;
			_statusCode = BAD_REQUEST;
			return (INCOMPLETE);
		}
		if (chunk.find_first_not_of("0123456789ABCDEF") != std::string::npos)
		{
			std::cerr << "Unexpected token" << std::endl;
			_statusCode = BAD_REQUEST;
			return (INCOMPLETE);
		}
		if (chunk == "0")
			break;
		size = std::strtoul(chunk.c_str(), NULL, 16);
		if (!size || size == ULONG_MAX)
		{
			std::cerr << "BAD REQUEST" << std::endl;
			_statusCode = BAD_REQUEST;
			return (INCOMPLETE);
		}
		_body += getNextWord(size);
		_size += size;
	}
	return ((_requestStatus = COMPLETE));
}

void	Request::parseBody()
{
	if (_requestStatus == COMPLETE)
		return ;
	if (_chunked == true)
		checkChunk();
	else
	{
		_body = _request;
		if (_body.size() == _size)
			_requestStatus = COMPLETE;
	}
}

bool	Request::isHeader(const std::string& headerName)
{
	StringMap::const_iterator	ite;

	ite = _header.find(headerName);
	return (ite != _header.end());
}

void	Request::ContentLength()
{
	std::string	ContentLength;
	size_t	size;
	if (_header.find("Content-Length") == _header.end())
		return ;
	ContentLength = _header["Content-Length"];
	if (ContentLength.find_first_not_of("0123456789") != std::string::npos)
	{
		_statusCode = BAD_REQUEST;
		throw Error("400 Bad Request");
	}
	if (ContentLength == "0")
	{
		_size = 0; // body is empty!
		return ;
	}
	size = std::strtoul(ContentLength.c_str(), NULL, 10); // str to unsigned long
	if (!size || size == ULONG_MAX)
	{
		_statusCode = BAD_REQUEST;
		throw Error("400 Bad Request");
	}
	_size = size;
}

int	Request::parse()
{
	try
	{
		if (_request.find("\r\n\r\n") == std::string::npos)
		{
			/* Header is incomplete */
			return (_requestStatus);
		}

		if (_chunked && _requestStatus != COMPLETE)
			checkChunk();
		else
		{
			if (!_headerParsed)
				FuncForParseHeader(); 
			parseBody();
		}
	}
	catch (std::exception &e)
	{
		_requestStatus = COMPLETE;
		std::cerr << e.what() << std::endl;
	}
	return (_requestStatus);
}

/*	GETTER	*/
size_t Request::getNextWord(std::string& word, const std::string& delimiter)
{
    size_t pos = _request.find(delimiter);
	size_t total;
    if (pos == std::string::npos)
    {
        // si'on trouve pas delimiteur
        throw Error("No delimiter");
    }
    word = _request.substr(0, pos);
	total = pos + delimiter.length();
	_payloadsize += total;
    _request.erase(0, pos + delimiter.length());
    return pos;
}

std::string Request::getNextWord(size_t sizeWord)
{
    if (_request.length() < sizeWord)
    {
        throw std::runtime_error("String length is shorter than the specified size");
    }
    std::string nextWord = _request.substr(0, sizeWord);
    _request.erase(0, sizeWord);
    _payloadsize += sizeWord + 2;
    std::cerr << "'" << GREEN << nextWord << "'" << RESET << std::endl;
    return nextWord;
}


std::string		Request::getHeader(const std::string& headerName)
{
	return (_header.find(headerName) != _header.end() ? _header[headerName] : "");
}

bool        Request::isHttpMethod(std::string const& str) const
{
	std::map<MethodType, std::string>::const_iterator ite;

	for (ite = _methods.begin(); ite != _methods.end(); ite++)
	{
		if (ite->second == str)
			return (true);
	}
	return (false);
}

int	Request::getStatusCode() const { return (_statusCode); }void	Request::FuncForParseHeader()
{

	Request::listFuncForParse::const_iterator	func;
	for (func = _funcforparse.begin();
		_requestStatus != COMPLETE && func != _funcforparse.end();

			func++)
	{
		(this->**func)();
	}
}

std::string	Request::getMethod() const {return (_method);}

std::string	Request::getPath() const { return (_path); }

std::string	Request::getBody() const { return (_body); }

size_t		Request::getPayloadSize() const { return (_payloadsize); }

std::string	Request::getQuery() const { return (_query); }

std::string	Request::getProtocolHTTP() const { return (_protocolHTTP); }

size_t		Request::getSize() const { return (_size); }

std::string	Request::getHost() const { return (_host); }

int			Request::getPort() const { return (_port); }

void	Request::PrintHeader()
{
	StringMap::iterator ite;
	std::string	mtd;

//	if (getMethod() == 0)
//		mtd = "GET";
//	else if (getMethod() == 1)
//		mtd = "POST";
//	else if (getMethod() == 1)
//		mtd = "DELETE";
//	else
//		mtd = "UNKNOWN";
	std::cout << PURPLE << "Request parsing check" <<  WHITE << std::endl;
	std::cout << "status code	: " << getStatusCode() << std::endl;
//	std::cout << "Method type	: " << mtd << std::endl;
	std::cout << "Method type	: " << getMethod() << std::endl;
	std::cout << "ProtocolHTTP	: " << getProtocolHTTP() << std::endl;
	std::cout << "Path			: " << getPath() << std::endl;
	std::cout << "host			: " << getHost() << std::endl;
	for (ite = _header.begin(); ite != _header.end(); ite++)
		std::cout << ite->first << "	: " << ite->second << std::endl;
	std::cout << "port			: " << getPort() << std::endl;
	std::cout << "Query			: " << getQuery() << std::endl;
	std::cout << "Size			: " << getSize() << std::endl;
	std::cout << "PayloadSize	: " << getPayloadSize() << std::endl;
	std::cout << "Body			: " << getBody() << std::endl;
}