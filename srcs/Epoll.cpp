#include "Webserv.hpp"

void	Webserv::initEvent(struct epoll_event &event, uint32_t flag, int fd)
{
	memset(&event, 0, sizeof(event));
	event.events = flag;
	event.data.fd = fd;
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
	_clients.push_back(client);
	return (_clients.size() - 1);
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

void Webserv::editSocket(int socket, uint32_t flag, struct epoll_event event)
{
	memset(&event, 0, sizeof(epoll_event));
	event.data.fd = socket;
	event.events = flag;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, socket, &event) < 0) // renouveller la mode
		throw EpollCtlException();
	// std::cout << YELLOW << "Success:" << RESET << " Socket event modified. Socket FD: " << socket << ", Event Flag: " << flag << std::endl;
}

void Webserv::removeSocket(int socket)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, socket, 0) < 0)
		throw EpollCtlException();
}

void Webserv::eraseClient(int index)
{
	int clientfd = _clients[index]->getSocket();

	removeSocket(clientfd);
	if (close(clientfd) < 0)
		std::cerr << "eraseClient(close) error" << std::endl;
	_clients.erase(_clients.begin() + index);
	delete _clients[index];
	// std::cout << YELLOW << "[Close]" << RESET << " connection on socket " + to_string(clientfd) << std::endl;
}

int	Webserv::routine(void)
{
	struct epoll_event	events[MAX_EPOLL_EVENTS];
	int 				nbEvents = 0;
	int					index = 0;

	// std::cout << PURPLE << std::setw(52) << "waiting for events" << RESET << std::endl;
	if ((nbEvents = epoll_wait(_epollFd, events, MAX_EPOLL_EVENTS, -1)) < SUCCESS)
		return (FAILED);

	for (int i = 0; i < nbEvents; i++)
	{
		if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
			return (close(events[i].data.fd), SUCCESS);
		if (events[i].data.fd == STDIN_FILENO) // NOT WORKING !
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return (FAILED);
		}
		else if ((index = findClientIndex(events[i].data.fd)) == FAILED) // si le client n'existe pas encore
			index = initConnection(events[i].data.fd);

		handleRequest(_clients[index], events[i]);
		if (_clients[index]->getRequest() == NULL)
		{
			eraseClient(index);
			continue;
		}
		Request *request = _clients[index]->getRequest();
		std::cout << "> " GREEN "[" << request->getMethod() << "] " BLUE "File requested is " << request->getPath() << RESET << std::endl;
		handleResponse(_clients[index], request, events[i]);
		StringMap::iterator it = request->_header.find("connection");
		if (it == request->_header.end() || it->second != "keep-alive")
			eraseClient(index);
	}
	return (SUCCESS);
}

int	Webserv::connectEpollToSockets()
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

void	Webserv::handleRequest(Client *client, struct epoll_event &event)
{
	(void)event;
	std::string	str = readFd(client->getSocket());
	if (str.empty())
		return;
	client->parse(str);
	if (client->getRequest()->_statusCode != OK)
		return;
	else
		editSocket(client->getSocket(), EPOLLIN, event);
	/*
		question: est-ce qu'on a pas besoin de mettre _client.erase
		dans chaque handles(request handle / response handle)?
	*/
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

bool Webserv::HandleCgi(Request &request, Client& client)
{
    CgiHandler cgi(request);
	std::string body;
	cgi.setEnv("SERVER_NAME", client._server->server_name);
    if (request._statusCode == NOT_FOUND)
		return (false);
	else
	{
		std::string output;
        if (cgi.getCgiOutput(output))
		{
			request.appendCgiBody(output);
			//std::cout << GREEN "CGI Executed" RESET<< std::endl;
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

void Webserv::handleResponse(Client *client, Request *req, struct epoll_event &event)
{
	(void)event;

	std::cout << "> Handling response" << std::endl;
	if (req == NULL)
		return;
	if (req->_statusCode != OK) // si une erreur est survenue, renvoyer la page d'erreur
		return (client->displayErrorPage(_statusCodeList.find(req->_statusCode)));
	std::pair<bool, std::vector<std::string> > cgi = isValidCGI(req->getRoot(), *client);	
	if (cgi.first) // is CGI valid or not
	{
		std::vector<std::string>::iterator it = cgi.second.begin();
		for (; it != cgi.second.end(); it++)
		{
			req->setRoot(*it); // set new root path
			if (!HandleCgi(*req, *client))
				return (client->displayErrorPage(_statusCodeList.find(req->_statusCode)));
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
		if (req->getMethod() == "GET")
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