#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "webserv.hpp"

class Location
{
	public:
		Location(/* args */);
		~Location();
		void				printLocation(void) const;
		static MethodType	methodType(std::string str);
		std::string			getRoot(void) const;
		std::string			getPath(void) const;
		std::string			getCgi(std::string extension) const;

	public:
		int									_client_body_limit;
		std::vector<MethodType>				_allowMethods;
		std::vector<std::string>			_index;
		std::string							_path;
		std::string							_root;
		std::map<std::string, std::string>	_cgi;
};

#endif
