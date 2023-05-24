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
	_statusCode = to_string(INCOMPLETE);
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
    _funcforparse.push_back(&_parseMethod);
    _funcforparse.push_back(&_parsePath);
    _funcforparse.push_back(&_parseHttpProtocol);// pas encore
	_funcforparse.push_back(&_parseHeaders);// pas encore
	_funcforparse.push_back(&_checkHeaders);// pas encore
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
		throw (BAD_REQUEST);
	if (path.length() > 2000)// ou bien 3000? max longuer de URL
		throw (URI_TOO_LONG);
	_path = path;
}

size_t Request::getNextWord(std::string& word, const std::string& delimiter)
{
    size_t pos = _request.find(delimiter);
    if (pos == std::string::npos)
    {
        // si'on trouve pas delimiteur
        throw std::runtime_error("No delimiter");
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
        throw std::runtime_error("String length is shorter than the specified size");
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

void	Request::FuncForParse()
{
	Request::listFuncForParse::const_iterator	func;

	for (func = _funcforparse.begin();
		_statusCode != COMPLETE && func != _funcforparse.end();
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
			return (atoi(_statusCode.c_str()));
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
	return (atoi(_statusCode.c_str()));
}