/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahocine <ahocine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/05/06 22:37:04 by ahocine           #+#    #+#             */
/*   Updated: 2023/05/06 22:37:29 by ahocine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

#	define	OK							200
#	define	CREATED						201
#	define	ACCEPTED					202
#	define	NO_CONTENT					204
#	define	MOVED_PERMANENTLY			301
#	define	FOUND						302
#	define	SEE_OTHER					303
#	define	NOT_MODIFIED				304
#	define	TEMPORARY_REDIRECT			307
#	define	PERMANENT_REDIRECT			308
#	define	BAD_REQUEST					400
#	define	UNAUTHORIZED				401
#	define	FORBIDDEN					403
#	define	NOT_FOUND					404
#	define	METHOD_NOT_ALLOWED			405
#	define	NOT_ACCEPTABLE				406
#	define	REQUEST_TIMEOUT				408
#	define	CONFLICT					409
#	define	GONE						410
#	define	LENGTH_REQUIRED				411
#	define	PAYLOAD_TOO_LARGE			413
#	define	URI_TOO_LONG				414
#	define	UNSUPPORTED_MEDIA_TYPE		415
#	define	RANGE_NOT_SATISFIABLE		416
#	define	EXPECTATION_FAILED			417
#	define	TOO_MANY_REQUESTS			429
#	define	INTERNAL_SERVER_ERROR		500
#	define	NOT_IMPLEMENTED				501	
#	define	BAD_GATEWAY					502
#	define	SERVICE_UNAVAILABLE			503
#	define	GATEWAY_TIMEOUT				504
#	define	HTTP_VERSION_NOT_SUPPORTED	505


# define CRLF			"\r\n"
# define SEP			" \t\n"

// STYLE
# define RESET			"\033[0m"
# define BOLD			"\033[1m"
# define UNDERLINE		"\033[4m"
# define BLINK			"\033[5m"
// COLORS
# define BLACK			"\033[30m"
# define RED			"\033[31m"
# define GREEN			"\033[32m"
# define YELLOW			"\033[33m"
# define BLUE			"\033[34m"
# define PURPLE			"\033[35m"
# define CYAN			"\033[36m"
# define WHITE			"\033[37m"

class Server;

enum MethodType
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};

typedef std::vector<Server>					ServerVector;
typedef std::vector<MethodType>				MethodVector;
typedef std::vector<std::string>			StrVector;
typedef std::map<int, std::string>			StatusMap;
typedef std::map<std::string, std::string>	StringMap;
typedef std::map<std::string, Server*>		ServerMap;

StrVector split(std::string input, char delimiter);
MethodType	strToMethodType(std::string str);
std::string methodTypeToStr(MethodType method);
std::string	trimSpacesStr(std::string *str);

template <typename T>
std::string to_string(T input)
{
	std::stringstream ss;
	ss << input;
	return ss.str();
}

#endif