/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtsuji <mtsuji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 12:01:33 by mtsuji            #+#    #+#             */
/*   Updated: 2023/06/19 12:01:40 by mtsuji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

/*----- INIT -----*/
void Webserv::initEvent(struct epoll_event &event, uint32_t flag, int fd)
{
	memset(&event, 0, sizeof(event));
	event.events = flag;
	event.data.fd = fd;
}

int	Webserv::connectEpollToSockets(void)
{
	struct epoll_event event;
	int ret;

	_epollFd = epoll_create(MAX_EPOLL_EVENTS);
	if (_epollFd < SUCCESS)
		throw EpollCreateException();
	for (size_t i = 0; i < _serversVec.size(); i++)
	{
		initEvent(event, EPOLLIN, _serversVec[i]._socket);
		ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serversVec[i]._socket, &event);
		char buffer;
		ssize_t result = read(_serversVec[i]._socket, &buffer, 0);
		if (result < 0)
			std::cout << "read error on socket " << _serversVec[i]._socket << std::endl;
		if (ret < SUCCESS)
			throw EpollCtlException();
	}

	initEvent(event, EPOLLIN, STDIN_FILENO);
	ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	if (ret < SUCCESS)
		throw EpollCtlException();
	return (SUCCESS);
}

int	Webserv::initConnection(int socket)
{
	struct epoll_event	event;
	Client 				*client = new Client(_serversMap[socket]);

	int newSocket = accept(socket, &client->_addr, &client->_addrLen);
	if (client->setSocket(newSocket) < SUCCESS && !(errno == EAGAIN || errno == EWOULDBLOCK))
		throw AcceptException();
	initEvent(event, EPOLLIN, client->getSocket());
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, client->getSocket(), &event) < SUCCESS)
		throw EpollCtlException();
	client->setTimer();
	_clients.push_back(client);
	return (_clients.size() - 1);
}
/*----- EPOLL -----*/
int	Webserv::findClientIndex(int socket)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->getSocket() == socket) // trouver UN client connecté au bon serveur
			return (i);
	}
	return (FAILED);
}

int	Webserv::routine(void)
{
	struct epoll_event	events[MAX_EPOLL_EVENTS];
	int 				nbEvents = 0;
	int					index = 0;
	Request				*request;

	if ((nbEvents = epoll_wait(_epollFd, events, MAX_EPOLL_EVENTS, 200)) < SUCCESS)
		return (FAILED);
	if (nbEvents == 0)
		checkTimeout();
	for (int i = 0; i < nbEvents; i++)
	{
		if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
			return (close(events[i].data.fd), SUCCESS);
		if (events[i].data.fd == STDIN_FILENO) // ignore les entrées clavier
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return (FAILED);
		}
		else if ((index = findClientIndex(events[i].data.fd)) == FAILED) // si le client n'existe pas encore
			index = initConnection(events[i].data.fd);

		handleRequest(_clients[index], events[i]);
		if ((request = _clients[index]->getRequest()) == NULL) // si la requête n'est pas encore complète
			continue;
		std::cout << "> " GREEN "[" << request->getMethod() << "] " BLUE "File requested is " << request->getPath() << RESET << std::endl;
		handleResponse(_clients[index], request, events[i]);
		// StringMap::iterator it = request->_header.find("connection");
		_toDelete.push_back(request);
	}
	return (SUCCESS);
}

/*----- SOCKET -----*/
void Webserv::editSocket(int socket, uint32_t flag, struct epoll_event event)
{
	memset(&event, 0, sizeof(epoll_event));
	event.data.fd = socket;
	event.events = flag;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, socket, &event) < 0) // renouveler le mode
		throw EpollCtlException();
}

void Webserv::removeSocket(int socket)
{
	std::cout << YELLOW << "remove socket: " << socket << RESET << std::endl;
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, socket, 0);
}

void Webserv::eraseClient(int index)
{
	int clientfd = _clients[index]->getSocket();

	removeSocket(clientfd);
	close(clientfd);
	if (_clients[index])
		delete _clients[index];
	_clients.erase(_clients.begin() + index);
}

/*----- UPLOAD -----*/
bool Webserv::isMultipartFormData(Request &request)
{
	return (request.getHeader("content-type").find("multipart/form-data") == std::string::npos) ? false : true;
}

bool Webserv::getBoundary(std::string contentType, std::string &boundary)
{
	size_t pos;
	int last;

	pos = contentType.find("boundary=");
	if (pos == std::string::npos)
		return (false);
	contentType.erase(0, pos + 9);//after "boundary="
	boundary = contentType.substr(0, contentType.find(CRLF));
	boundary.erase(boundary.find_last_not_of("\t") + 1);
	last = boundary.length() - 1;
	if (boundary[0] == '\"' && boundary[last] == '\"')
	{
		boundary.erase(0, 1);
		boundary.erase(last - 1);
	}
	if (boundary.length() > 70)//max len of bounrary = 70 (RFC2046 for HTTP1.1)
		return (false);
	boundary = "--" + boundary;
	return (true);
}

size_t Webserv::getfield(std::string content, const std::string &field, std::string *name)
{
	size_t pos;

	pos = content.find(field);
	if (pos == std::string::npos)
		return (*name = "", pos);
	pos += field.size();
	content.erase(0, pos);
	*name = content.substr(0, content.find("\""));
	return (pos);
}

void Webserv::upload_path(Client &client, std::string &path, Request &request, size_t pos)
{
    std::string pwd(PWD);
    std::string uploadpath = client._server->root;

    if (!uploadpath.empty() && uploadpath[0] == '.')
        uploadpath = uploadpath.substr(1);
	if (uploadpath[uploadpath.length() - 1] != '/')
		uploadpath += "/";
	uploadpath += "uploads";
	std::size_t i;

	while ((i = path.find(" ")) != std::string::npos)
		path.replace(i, 1, "_");
	if (uploadpath[uploadpath.length() - 1] != '/')
		uploadpath += '/';

	std::string command = "mkdir -p " + pwd + uploadpath;
	std::system(command.c_str());
	command = "chmod 777 " + pwd + uploadpath;
	std::system(command.c_str());
	uploadpath += path;
	path = uploadpath;
	if (!path.empty() && path[0] != '.')
        path.insert(0, ".");
	request.insertUploadpath(pos, uploadpath);
}

void Webserv::writeContent(Request &request, const std::string &path, const std::string &content)
{
	std::ofstream 	file;
	std::string 	pathDir;
	std::string 	filename;
	std::string 	newfile;
	size_t 			pos_separator;
	std::string		pwd(PWD);

	pos_separator = path.rfind("/");
	pathDir = path.substr(0, pos_separator + 1);
	filename = path.substr(pos_separator + 1);
	if (filename.empty())
	{
		request._statusCode = BAD_REQUEST;
		return ;
	}
	else if (AccessiblePath(path))
		newfile = pwd + generateCopyFile(pathDir, filename);
	else
	{
		if (!path.empty() && path[0] == '.')
		{
			newfile = pwd;
			newfile.append(path, 1);
		}
		else
			newfile = pwd + path;
	}
	request._statusCode = CREATED;
	file.open(newfile.c_str(), std::ios::out | std::ios::binary);
	if (!file.is_open())
	{
    	std::cerr << "Error: " << strerror(errno) << std::endl;
		request._statusCode = INTERNAL_SERVER_ERROR;
		return ;
	}
	file << content;
	if (file.bad())
	{
		request._statusCode = INTERNAL_SERVER_ERROR;
		return ;
	}
	file.close();
}

void Webserv::handleMultipart(Request &request, Client &client, std::string *filepath)
{
	std::string	boundary;
	size_t		pos;
	size_t		pos_name;
	size_t		pos_file = 0;
	std::string	name;
	std::string filename;
	std::string content;
	std::string crlf(CRLF);

	std::string body = request.getBody();
	std::string contentdispositon;

	 if (!getBoundary(request.getHeader("content-type"), boundary))
		return (request._statusCode = BAD_REQUEST, void());
	while (body.find(boundary + CRLF) != std::string::npos)
	{
		pos_file += body.find(boundary + CRLF) + boundary.length() + 2;
		body.erase(0, body.find(boundary + CRLF) + boundary.length() + 2);
		pos = body.find("Content-Disposition:");
		if (pos == std::string::npos)
			return (request._statusCode = BAD_REQUEST, void());
		contentdispositon = body.substr(pos, body.find(CRLF));
		pos_name = getfield(contentdispositon, "name=\"", &name);
		pos_file += getfield(contentdispositon, "filename=\"", &filename);
		body.erase(0, body.find(crlf + crlf) + 4);
		content = body.substr(0, body.find(CRLF + boundary));
	}
	
	if (filename != "")
	{
		upload_path(client, filename, request, pos_file);
		writeContent(request, filename, content);
	}
	//if (filename != "")
	//{
	//	upload_path(client, filename, request, pos_file);
	//}
	std::cout << BLUE << "name is:\t" << name << RESET << std::endl;
	std::cout << BLUE << "filename is:\t" << filename << RESET << std::endl;
	filepath = &filename;
}


/*----- CGI -----*/
void Webserv::CgihandleMultipart(Request &request, Client &client)
{
	std::string	boundary;
	size_t		pos;
	size_t		pos_name;
	size_t		pos_file = 0;
	std::string	name;
	std::string filename;

	std::string body = request.getBody();
	std::string contentdispositon;

	 if (!getBoundary(request.getHeader("content-type"), boundary))
		return (request._statusCode = BAD_REQUEST, void());
	while (body.find(boundary + CRLF) != std::string::npos)
	{
		pos_file += body.find(boundary + CRLF) + boundary.length() + 2;
		body.erase(0, body.find(boundary + CRLF) + boundary.length() + 2);
		pos = body.find("Content-Disposition:");
		if (pos == std::string::npos)
			return (request._statusCode = BAD_REQUEST, void());
		contentdispositon = body.substr(pos, body.find(CRLF));
		pos_name = getfield(contentdispositon, "name=\"", &name);
		pos_file += getfield(contentdispositon, "filename=\"", &filename);
	}
	if (filename != "")
	{
		upload_path(client, filename, request, pos_file);
	}
	std::cout << BLUE << "name is:\t" << name << RESET << std::endl;
	std::cout << BLUE << "filename is:\t" << filename << RESET << std::endl;
}

bool Webserv::HandleCgi(Request &request, Client& client)
{
	if (request.getMethod() == "POST" && isMultipartFormData(request))
		CgihandleMultipart(request, client);
	CgiHandler cgi(request);
	cgi.setEnv("SERVER_NAME", client._server->server_name);
	cgi.setEnv("DOCUMENT_ROOT", "./html");
	if (request._statusCode == NOT_FOUND || request._statusCode == BAD_REQUEST)
		return (false);
	std::string output = request.getBody();
	if (cgi.getCgiOutput(output))
		return (request.appendCgiBody(output), true);
	std::cout << RED "ERROR CGI EXECUTION" << std::endl;
	request._statusCode = INTERNAL_SERVER_ERROR;
	return (false);
}

/*-----REQUEST / RESPONSE-----*/
void Webserv::handleRequest(Client *client, struct epoll_event &event)
{
	std::string	str = readFd(client->getSocket());
	if (str.empty())
		return ;
	client->parse(str);
	client->setTimer();
	if (client->getRequest()->_statusCode != OK)
		return (editSocket(client->getSocket(), EPOLLIN, event));
	else
		editSocket(client->getSocket(), EPOLLIN, event);
}

void Webserv::handleResponse(Client *client, Request *req, struct epoll_event &event)
{
	(void)event;

	std::cout << "> Handling response" << std::endl;
	if (req == NULL)
		return ;
	if (req->_statusCode != OK) // si une erreur est survenue, renvoyer la page d'erreur
		return (client->displayErrorPage(_statusCodeList.find(req->_statusCode)));
	std::pair<bool, std::vector<std::string> > cgi = isValidCGI(*req, *client);	
	if (cgi.first) // is CGI valid or not
	{
		std::vector<std::string>::iterator it = cgi.second.begin();
		for (; it != cgi.second.end(); it++)
		{
			req->setRoot(*it); // set new root path
			if (!HandleCgi(*req, *client))
				return (eraseTmpFile(cgi.second), client->displayErrorPage(_statusCodeList.find(req->_statusCode)));
		}
		std::cout << CYAN "CGI BOOL IS TRUE" RESET << std::endl;
		if (req->getMethod() == "GET")
			CgiGetMethod(*client, req);
		else if (req->getMethod() == "POST")
			CgiPostMethod(*client, req);
		else
			return (eraseTmpFile(cgi.second), client->displayErrorPage(_statusCodeList.find(METHOD_NOT_ALLOWED)));
		eraseTmpFile(cgi.second);
	}
	else
	{
		if (client->_server->redirect_status != -1)
			redirectMethod(*client, *req);
		else if (req->getMethod() == "GET")
			getMethod(*client, req->getPath());
		else if (req->getMethod() == "POST")
		{
			std::string filepath;
			if (isMultipartFormData(*req))
			{
				handleMultipart(*req, *client, &filepath);
				if (req->_statusCode == NOT_FOUND || req->_statusCode == BAD_REQUEST)
					(client->displayErrorPage(_statusCodeList.find(METHOD_NOT_ALLOWED)));
			}
			postMethod(*client, filepath);
		}
		else if (req->getMethod() == "DELETE")
			deleteMethod(*client, req->getPath());
		else
			return (client->displayErrorPage(_statusCodeList.find(METHOD_NOT_ALLOWED)));
	}
	std::cout << std::endl;
}



/*----- EXCEPTION -----*/
const char *Webserv::EpollCreateException::what() const throw()
{
	return ("Error: Epoll_create() failed");
}

const char *Webserv::EpollCtlException::what() const throw()
{
	return ("Error: Epoll_ctl() failed");
}

const char *Webserv::EpollWaitException::what() const throw()
{
	return ("Error: Epoll_wait() failed");
}

const char *Webserv::AcceptException::what() const throw()
{
	return ("Error: Accept() failed");
}
