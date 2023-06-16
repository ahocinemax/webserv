#include "Webserv.hpp"

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
		initEvent(event, EPOLLIN | EPOLLET, _serversVec[i]._socket);
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

/*----- CGI-----*/
bool Webserv::isMultipartFormData(Request &request)
{
	size_t pos;
	//std::cout << request.getHeader("content-type") << std::endl;//for check
	pos = request.getHeader("content-type").find("multipart/form-data");
	if (pos == std::string::npos)
		return (false);
	return (true);
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
	{
		*name = "";
		return (pos);
	}
	pos += field.size();
	content.erase(0, pos);
	*name = content.substr(0, content.find("\""));
	return (pos);
}

void Webserv::upload_path(Client &client, std::string &path, Request &request, size_t pos)
{
    std::string pwd(PWD);
    std::string uploadpath = client._server->root;

	if (uploadpath[uploadpath.length() - 1] != '/')
		uploadpath += "/";
	uploadpath += "uploads";
	std::size_t i;

	while ((i = path.find(" ")) != std::string::npos)
		path.replace(i, 1, "_");
	if (uploadpath[uploadpath.length() - 1] != '/')
		uploadpath += '/';

    std::string command = "mkdir -p " + pwd + uploadpath;
	command = "chmod 777 " + pwd + uploadpath;
    std::system(command.c_str());
	uploadpath += path;
	path = uploadpath;
	request.insertUploadpath(pos, uploadpath);
}

void Webserv::handleMultipart(Request &request, Client &client)
{
	std::string	boundary;
	size_t		pos;
	size_t		name_pos;
	size_t		pos_file = 0;
	std::string	name;
	std::string filename;

	std::string body = request.getBody();
	std::string contentdispositon;

	 if (!getBoundary(request.getHeader("content-type"), boundary))
	 {
		request._statusCode = BAD_REQUEST;
		return ;
	 }
	std::cout << YELLOW << "boundary is:\t" << boundary << RESET << std::endl;
	while (body.find(boundary + CRLF) != std::string::npos)
	{
		pos_file += body.find(boundary + CRLF) + boundary.length() + 2;
		body.erase(0, body.find(boundary + CRLF) + boundary.length() + 2);
		pos = body.find("Content-Disposition:");
		if (pos == std::string::npos)
		{
			request._statusCode = BAD_REQUEST;
			return ;
	 	}
		contentdispositon = body.substr(pos, body.find(CRLF));
		name_pos = getfield(contentdispositon, "name=\"", &name);
		pos_file += getfield(contentdispositon, "filename=\"", &filename);
	}
	//if (filename != "")
	//{
	//	upload_path(client, filename, request, pos_file);
	//}
	std::cout << BLUE << "name is:\t" << name << RESET << std::endl;
	std::cout << BLUE << "filename is:\t" << filename << RESET << std::endl;
}

bool Webserv::HandleCgi(Request &request, Client& client)
{
	if (request.getMethod() == "POST")
	{
		std::cout << "request body size (before parse)= " << request.getBody().size() << std::endl;
		if (isMultipartFormData(request))
			handleMultipart(request, client);
	}
	std::string body;
	CgiHandler cgi(request);
	cgi.setEnv("SERVER_NAME", client._server->server_name);
	cgi.setEnv("DOCUMENT_ROOT", "./html");
	if (request._statusCode == NOT_FOUND || request._statusCode == BAD_REQUEST)
		return (false);
	else
	{
		std::string output = request.getBody();
		if (cgi.getCgiOutput(output))
		{
			request.appendCgiBody(output);
			//std::cout << "cgi response: "<< request.getCgiBody(0) << std::endl;
		}
		else
		{
			std::cout << RED "ERROR CGI EXECUTION" << std::endl;
			request._statusCode = INTERNAL_SERVER_ERROR;
			return (false);
		}
	}
	return (true);
}

/*-----REQUEST / RESPONSE-----*/
void Webserv::handleRequest(Client *client, struct epoll_event &event)
{
	(void)event;
	std::string	str = readFd(client->getSocket());
	if (str.empty())
		return;
	client->parse(str);
	client->setTimer();
	if (client->getRequest()->_statusCode != OK)
		return;
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
		// std::cout << YELLOW "REQ BODY: " << req->getBody() << RESET << std::endl;
		std::vector<std::string>::iterator it = cgi.second.begin();
		for (; it != cgi.second.end(); it++)
		{
			req->setRoot(*it); // set new root path
			if (!HandleCgi(*req, *client))
				return (eraseTmpFile(cgi.second), client->displayErrorPage(_statusCodeList.find(req->_statusCode)));
		}
		std::cout << CYAN "CGI BOOL IS TRUE" RESET << std::endl;
		if (req->getMethod() == "GET")
			getCgiMethod(*client, req);
		else if (req->getMethod() == "POST")
			postCgiMethod(*client, req);
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
			postMethod(*client, *req);
		else if (req->getMethod() == "DELETE")
			deleteMethod(*client, req->getPath());
		else
			return (client->displayErrorPage(_statusCodeList.find(METHOD_NOT_ALLOWED)));
	}
	std::cout << std::endl;

	// editSocket(client->getSocket(), EPOLLIN, event);
}

int	Webserv::findClientIndex(int socket)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		// std::cout << _clients[i]->_server->_socket << std::endl;
		if (_clients[i]->getSocket() == socket) // trouver UN client connecté au bon serveur
			return (i);
		// if (_serversMap[socket]->_connectedClients (_clients[i]->_ipAdress)) // trouver LE client connecté au bon serveur
	}
	return (FAILED);
}

int	Webserv::routine(void)
{
	struct epoll_event	events[MAX_EPOLL_EVENTS];
	int 				nbEvents = 0;
	int					index = 0;

	if ((nbEvents = epoll_wait(_epollFd, events, MAX_EPOLL_EVENTS, 2000)) < SUCCESS)
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
		if (_clients[index]->getRequest() == NULL) // si la requête n'est pas encore complète
			continue;
		Request *request = _clients[index]->getRequest();
		std::cout << "> " GREEN "[" << request->getMethod() << "] " BLUE "File requested is " << request->getPath() << RESET << std::endl;
		handleResponse(_clients[index], request, events[i]);
		StringMap::iterator it = request->_header.find("connection");
		delete request;
	}
	return (SUCCESS);
}

void Webserv::editSocket(int socket, uint32_t flag, struct epoll_event event)
{
	memset(&event, 0, sizeof(epoll_event));
	event.data.fd = socket;
	event.events = flag;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, socket, &event) < 0) // renouveler le mode
		throw EpollCtlException();
	// std::cout << YELLOW << "Success:" << RESET << " Socket event modified. Socket FD: " << socket << ", Event Flag: " << flag << std::endl;
}

void Webserv::removeSocket(int socket)
{
	std::cout << YELLOW << "remove socket: " << socket << RESET << std::endl;
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, socket, 0) < 0)
		perror("epoll_ctl");
}

void Webserv::eraseClient(int index)
{
	int clientfd = _clients[index]->getSocket();

	removeSocket(clientfd);
	if (close(clientfd) < 0)
		std::cerr << "eraseClient(close) error" << std::endl;
	if (_clients[index])
		delete _clients[index];
	_clients.erase(_clients.begin() + index);
}

void Webserv::eraseClient(std::vector<Client*>::iterator index)
{
	(void)index;
	// int clientfd = (*index)->getSocket();

	// removeSocket(clientfd);
	// if (close(clientfd) < 0)
	// 	std::cerr << "eraseClient(close) error" << std::endl;
	// if (*index)
	// 	delete *index;
	// _clients.erase(index);
}

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
