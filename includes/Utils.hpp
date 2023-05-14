#ifndef UTILS_HPP
# define UTILS_HPP

# include <sstream>
# include <iostream>
# include <string>
# include <string.h>
# include <vector>
# include <map>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdlib.h>

# define BUFFER_SIZE	30720
# define SUCCESS		0
# define FAILED			-1
# define EMPTY			1

# define SEP			" \t\n"
# define RESET			"\033[0m"
# define BOLD			"\033[1m"
# define UNDERLINE		"\033[4m"
# define BLINK			"\033[5m"

# define BLACK			"\033[30m"
# define RED			"\033[31m"
# define GREEN			"\033[32m"
# define YELLOW			"\033[33m"
# define BLUE			"\033[34m"
# define PURPLE			"\033[35m"
# define CYAN			"\033[36m"
# define WHITE			"\033[37m"

class Server;

typedef std::vector<Server>	vecServer;

enum MethodType
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};
std::vector<std::string> split(std::string input, char delimiter);

#endif