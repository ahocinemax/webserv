#ifndef REQUEST_HPP
# define REQUEST_HPP

typedef std::map<std::string, std::string>	mapString;

class Request
{
	public:
		Request(/* args */);
		~Request();

		mapString	_header;

		std::size_t							_contentLength;
		std::string							_method;
		std::string							_path;
		std::string							_body;
};

#endif