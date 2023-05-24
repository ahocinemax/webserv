#include "Request.hpp"


/*
	sample:
	"GET / HTTP/1.1\r\n
	Host: localhost:8080\r\n
	Connection: keep-alive\r\n
	sec-ch-ua: \"Google Chrome\";v=\"113\", \"Chromium\";v=\"113\", \"Not-A.Brand\";v=\"24\"\r\n
	sec-ch-ua-mobile: ?0\r\n
	sec-ch-ua-platform: \"Linux\"\r\n
	Upgrade-Insecure-Requests: 1\r\n
	User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/113.0.0.0 Safari/537.36\r\n
	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,**;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n
	Sec-Fetch-Site: none\r\n
	Sec-Fetch-Mode: navigate\r\n
	Sec-Fetch-User: ?1\r\n
	Sec-Fetch-Dest: document\r\n
	Accept-Encoding: gzip, deflate, br\r\n
	Accept-Language: en-US,en;q=0.9,ja;q=0.8,fr;q=0.7\r\n
	Cookie: sessionId=yekwp6nv9y1po66y\r\n\r\n"

	->
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

*/
Request::Request(): _request(""), 
{
	initVariables();
	initFuncForParse();
}
void	Request::initVariables()
{
	_statusCode = OK;
	_requestStatus = INCOMPLETE
	_method = UNKNOWN;
	_path = "";
	_protocolHTTP = "";
	_size = 0;
	_body = "";
	_host = "";
	_port = 0;
	_payloadSize = 0;
	_headerParsed = false;
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
    _method = strToMethodType(method);
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
	if (path.length() > 2000)// ou bien 3000? max longuer de URL
	{
		_statusCode = URI_TOO_LONG;
		throw Error("414 URI Too Long");
	}
	_path = path;
}
void	Request::parseHttpProtocol()
{
	std::string protocolHTTP;
	
	_getNextWord(protocolHTTP, "\r\n");
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
		while (pos != std::string::npos && _request.find("\r\n"))
		{
			pos = getNextWord(headerName, ":");
			if (pos == std:;string::npos)
				break;
			getNextWord(headerVal, "\r\n");	
			trimSpacesStr(&headerVal);
			if (isHeader(headerName))
				throw Error("400 Bad Request");	//->this header is already existed
			_header[headerName] = headerVal;		
		}
		getNextWord(headerName, "\r\n");// separete between header/body
}

void	Request::checkHeaders()
{
	/*
		memo:
		il faut faire:
		-> parser entre host / port (localhost:8080)
		-> checker content-length
		enfin, bool _headerParsed = true
	*/
}

size_t Request::getNextWord(std::string& word, const std::string& delimiter)
{
    size_t pos = _request.find(delimiter);
    if (pos == std::string::npos)
    {
        // si'on trouve pas delimiteur
        throw Error("No delimiter");
    }
    word = _request.substr(0, pos);
    _request.erase(0, pos + delimiter.length());
    _payloadSize += pos + delimiter.length();
    return pos;
}

std::string Request::getNextWord(size_t sizeWord)
{
    if (_request.length() < sizeWord)
    {
        throw Error("String length is shorter than the specified size");
    }
    std::string nextWord = _request.substr(0, sizeWord);
    _request.erase(0, sizeWord);
	//_payloadsize : taille totale de data recues
    _payloadSize += sizeWord + 2;
    std::cerr << "'" << GREEN << nextWord << "'" << RESET << std::endl;
    return nextWord;
}


std::string		Request::GetHeader(const std::string& headerName)
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

bool	Request::isHeader(const std::string& headerName)
{
	StringMap::const_iterator	ite;

	ite = _header.find(headerName);
	return (ite != _header.end());
}

void	Request::FuncForParse()
{
	Request::listFuncForParse::const_iterator	func;

	for (func = _funcforparse.begin();
		_requestStatus != COMPLETE && func != _funcforparse.end();
			func++)
	{
		(this->**func)();
	}
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
		else
		{
			if (!_headerParsed)
				FuncForParse(); // +ajouter  parsing for body
		}
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return (_requestStatus);
}