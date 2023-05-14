#include "Webserv.hpp"

Webserv::Webserv(/* args */)
{
}

Webserv::~Webserv() {}

void	Webserv::deleteMethod(Client &client, std::string path)
{
	std::string		filePath = getPath(client, path);
	FILE			*file = fopen(filePath.c_str(), "r");

	if (file == NULL)
	{
		client.displayErrorPage(_statutCode.find(404));
		return ;
	}
	fclose(file);
	
	std::remove(filePath.c_str());

	Response	response(_statutCode[200]);
	std::string	header = response.makeHeader();
	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);

	if (ret < 0)
		client.displayErrorPage(_statutCode.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statutCode.find(400));
	std::cout << GREEN "File \"" << filePath << "\" deleted" RESET << std::endl;
}

void	Webserv::postMethod(Client &client, Request &request)
{
	if (request._header["Transfer-Encoding"] != "chunked" && request._header.find("Content_Lenght") == request._header.end())
	{
		client.displayErrorPage(_statutCode.find(411));
		return ;
	}

	std::string		filePath = getPath(client, request._path);

	struct stat		fileStat;
	lstat(filePath.c_str(), &fileStat);
	if (S_ISDIR(fileStat.st_mode))
	{
		if (request._header.find("Content-Type") == request._header.end())
		{
			client.displayErrorPage(_statutCode.find(400));
			return ;
		}
		std::size_t	begin = request._header["Content-Type"].find("boundary=");
		if (begin == std::string::npos)
		{
			client.displayErrorPage(_statutCode.find(400));
			return ;
		}
		std::string	boundary = request._header["Content-Type"].substr(begin + 9);
		begin = 0;
		std::size_t	end = 0;
		std::string		fileName;
		while (true)
		{
			begin = request._body.find("name=", begin) + 6;
			end = request._body.find_first_of("\"", begin);
			if (begin == std::string::npos || end == std::string::npos)
				break ;
			fileName = request._body.substr(begin, end - begin);
			begin = request._body.find("\r\n\r\n", begin) + 4;
			end = request._body.find(boundary, begin);
			if (begin == std::string::npos || end == std::string::npos)
				break ;
			if (writeResponse(client, request._body.substr(begin, end - begin - 4), filePath + "/" + fileName) < 0)
				break ;
			if (request._body[end + boundary.length()] == '-')
				break ;
		}		
	}
	else
	{
		writeResponse(client, request._body, filePath);
	}
	int	code = 201;
	if (request._header["Content-Lenght"] == "0")
		code = 204;
	
	Response	response(_statutCode[code]);
	std::string	header = response.makeHeader();

	int			ret = send(client.getSocket(), header.c_str(), header.length(), 0);
	if (ret < 0)
		client.displayErrorPage(_statutCode.find(500));
	else if (ret == 0)
		client.displayErrorPage(_statutCode.find(400));
	std::cout << GREEN << filePath << " posted (" << code << ")" RESET << std::endl;
}

// void	Webserv::getMethod(Client &client, std::string path)
// {
	// ;
// }

std::string	Webserv::getPath(Client &client, std::string path)
{
	std::string		filePath = "";
	std::string		res;
	filePath.append(client.setRootPath(path));
	Location	*location = client._server->getLocation(path);
	if (location != NULL)
		res = location->getPath();
	else
		res = "";
	std::string str = path.substr(res.length());
	filePath.append(str);

	return (filePath);	
}

int	Webserv::writeResponse(Client &client, std::string body, std::string path)
{
	std::size_t	begin = path.find_last_of("/");
	std::string	fileName = path.substr(begin + 1);
	std::string	dirPath = path.substr(0, begin);

	std::string	command = "mkdir -p " + dirPath;
	system(command.c_str());
	int	fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		client.displayErrorPage(_statutCode.find(500));
		return (-1);
	}
	
}