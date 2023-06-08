#include "Request.hpp"

Request::Request(const std::string& request) : _request(request)
{
	initVariables();
	initFuncForParse();
}

Request::~Request() {}

void	Request::initVariables()
{
	_statusCode = NOT_FOUND; // mettre une erreur par defaut, puis la changer si besoin
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
	_root = "html";
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
		std::cerr << RED "Error:" RESET " Invalid Method" << std::endl;
		return ;
	}
	_method = method;
	_statusCode = OK;
}

void	Request::parsePath()
{
	std::string		path;
	size_t			pos;

	getNextWord(path, " ");
	if (path == "" || path[0] != '/')
	{
		_statusCode = BAD_REQUEST;
		return ;
	}
	if (path.length() > 2000) // ou bien 3000? max longuer de URL
	{
		_statusCode = URI_TOO_LONG;
		return ;
	}
	pos = path.find("?");
	if (pos != std::string::npos)
	{
		_query = path.substr(pos + 1);
		path.erase(pos);
	}
	_path = path;
	_root += path;
	_statusCode = OK;
}

void	Request::parseHttpProtocol()
{
	std::string protocolHTTP;
	
	getNextWord(protocolHTTP, CRLF);
	if (protocolHTTP.find("HTTP") == std::string::npos)
	{
		_statusCode = BAD_REQUEST;
		return ;
	}
	if (protocolHTTP != "HTTP/1.1")
	{
		_statusCode = HTTP_VERSION_NOT_SUPPORTED;
		return ;
	}
	_protocolHTTP = protocolHTTP;
	_statusCode = OK;
}

void	Request::parseHeaders()
{
		std::string	headerName;
		std::string headerVal;
		size_t pos = 0;
		while (pos != std::string::npos && _request.find(CRLF))
		{
			pos = getNextWord(headerName, ":");
			if (pos == std::string::npos)
				break;
			toLower(&headerName);
			getNextWord(headerVal, CRLF);	
			trimSpacesStr(&headerVal);
			if (isHeader(headerName))
				_statusCode = BAD_REQUEST;
			_header.insert(std::make_pair(headerName, headerVal));
			if (_payloadsize > 10485759)
				_statusCode = PAYLOAD_TOO_LARGE;
		}
		getNextWord(headerName, CRLF);
		_statusCode = OK;
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
	StringMap::const_iterator ite;
	if (!parseHeaderHost())
	{
		_statusCode = BAD_REQUEST;
		return ;
	}
	ContentLength();
	if (_method != "POST") //GET et DELETE n'ont pas besoin de body
	{
		_headerParsed = true;
		return ;
	}
	ite = _header.find("transfer-encoding");
	if (ite != _header.end() && ite->second.find("chunked"))
		_chunked = true;
	if (_header.find("content-length") == _header.end())
	{
		_statusCode = BAD_REQUEST;
		return ;
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
			std::cerr << RED "Error:" RESET " Unexpected token" << std::endl;
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
	if (_header.find("content-length") == _header.end())
		return ;
	ContentLength = _header["content-length"];
	if (ContentLength.find_first_not_of("0123456789") != std::string::npos)
	{
		_statusCode = BAD_REQUEST;
		return ;
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
		return ;
	}
	_size = size;
}

void	Request::appendCgiBody(const std::string &output) { _cgibody.push_back(output); }

void	Request::parse()
{
	if (_request.find("\r\n\r\n") == std::string::npos)
	{
		std::cerr << RED "Error:" RESET " Request Header is incomplete" << std::endl;
		return ;
	}
	if (_chunked && _requestStatus != COMPLETE)
		checkChunk();
	else
	{
		if (!_headerParsed)
			FuncForParseHeader();
		if (_statusCode == OK)
			parseBody();
	}
}

/*	GETTER	*/
size_t Request::getNextWord(std::string& word, const std::string& delimiter)
{
    size_t pos = _request.find(delimiter);
	size_t total;
    if (pos == std::string::npos)
    {
		std::cerr << RED "Error:" RESET "No delimiter" << std::endl;
		return 0;
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
		std::cerr << RED "Error:" RESET "String length is shorter than the specified size" << std::endl;
		return NULL;
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

int	Request::FuncForParseHeader()
{

	Request::listFuncForParse::const_iterator	func;
	for (func = _funcforparse.begin();
		_requestStatus != COMPLETE && func != _funcforparse.end();
			func++)
	{
		(this->**func)();
		if (_statusCode != OK)
			return (FAILED);
	}
	_statusCode = OK;
	return (SUCCESS);
}

std::string	Request::getMethod() const {return (_method);}

std::string	Request::getPath() const { return (_path); }

std::string	Request::getRoot() const { return (_root); }

std::string	Request::getBody() const { return (_body); }

std::string	Request::getCgiBody(int index) const { return (_cgibody[index]); }

size_t		Request::getPayloadSize() const { return (_payloadsize); }

std::string	Request::getQuery() const { return (_query); }

std::string	Request::getProtocolHTTP() const { return (_protocolHTTP); }

size_t	Request::getSize() const { return (_size); }

std::string	Request::getHost() const { return (_host); }

int	Request::getPort() const { return (_port); }

void	Request::setRoot(std::string& root)
{ 
	_root = root; 
}

void	Request::setCgiBody(std::string& body)
{ 
	_cgibody.push_back(body); 
}

void	Request::PrintHeader()
{
	StringMap::iterator ite;
	std::string	mtd;

	std::cout << PURPLE << "> Request parsing check" <<  BLUE << std::endl;
	std::cout << "status code	: " << _statusCode << std::endl;
	std::cout << "Method type	: " << getMethod() << std::endl;
	std::cout << "ProtocolHTTP	: " << getProtocolHTTP() << std::endl;
	std::cout << "Path			: " << getPath() << std::endl;
	std::cout << "Root			: " << getRoot() << std::endl;
	std::cout << "host			: " << getHost() << std::endl;
	for (ite = _header.begin(); ite != _header.end(); ite++)
		std::cout << ite->first << "	: " << ite->second << std::endl;
	std::cout << "port			: " << getPort() << std::endl;
	std::cout << "Query			: " << getQuery() << std::endl;
	std::cout << "Size			: " << getSize() << std::endl;
	std::cout << "PayloadSize	: " << getPayloadSize() << std::endl;
	std::cout << "Body			: " << getBody() << RESET << std::endl;
}
