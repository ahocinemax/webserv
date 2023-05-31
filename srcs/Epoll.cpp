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
	Client				client(_serversMap[socket]);

	if ((client.setSocket(accept(socket, &client._addr, &client._addrLen)) < SUCCESS && !(errno == EAGAIN || errno == EWOULDBLOCK)))
		throw AcceptException();

	initEvent(event, EPOLLIN, client.getSocket());
	std::cout << YELLOW << "[Accept]" << RESET << " connection on socket " + to_string(client._server->_socket) + " at " + client._server->_ipAddress + ":" + client._server->_port << std::endl;
	std::cout << PURPLE << std::setw(52) << "socket " + to_string(client.getSocket()) + " created to communicate" << RESET << std::endl;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, client.getSocket(), &event) < SUCCESS)
		throw EpollCtlException();
	_clients.push_back(client);
	return (_clients.size() - 1);
}

int	Webserv::findClientIndex(int socket)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].getSocket() == socket)
			return (i);
	}
	return (FAILED);
}

/*
	editSocket:
	Modification de flag epoll
	-> request	=	mode "read"
	-> response	=	mode "write"
*/
void Webserv::editSocket(int socket, uint32_t flag, struct epoll_event event)
{
	//struct epoll_event event;

	memset(&event, 0, sizeof(epoll_event));
	event.data.fd = socket;
	event.events = flag;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, socket, &event) < 0) // renouveller la mode
		throw EpollCtlException();	
	std::cout << YELLOW << "Success:" << RESET << " Socket event modified. Socket FD: " << socket << ", Event Flag: " << flag << std::endl;
}

void Webserv::removeSocket(int socket)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, socket, 0) < 0)
		throw EpollCtlException();
}

void Webserv::eraseClient(int index)
{
	int clientfd = _clients[index].getSocket();

	removeSocket(clientfd);
	if (close(clientfd) < 0)
		std::cerr << "eraseClient(close) error" << std::endl;
	_clients.erase(_clients.begin() + index);
	std::cout << YELLOW << "[Close]" << RESET << " connection on socket " + to_string(clientfd) << std::endl;
}

int	Webserv::routine(void)
{
	struct epoll_event	events[MAX_EPOLL_EVENTS];
	int					nbEvents = 0;
	int					index = 0;

	std::cout << PURPLE << std::setw(52) << "waiting for events" << RESET << std::endl;
	if ((nbEvents = epoll_wait(_epollFd, events, MAX_EPOLL_EVENTS, -1)) < SUCCESS)
		throw EpollWaitException();

	for (int i = 0 ; i < nbEvents ; i++)
	{
		std::cout << PURPLE "handling event on socket " << events[i].data.fd << RESET << std::endl;
		if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
			return (close(events[i].data.fd), SUCCESS);
		if ((index = findClientIndex(events[i].data.fd)) == FAILED) // si le client n'existe pas encore
			index = initConnection(events[i].data.fd);
		else if (events[i].data.fd == STDIN_FILENO) // NOT WORKING !
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return (FAILED);
		}
		handleRequest(_clients[index], events[i]);
		Request *request = _clients[index].getRequest();
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
		/*check sserverVec[num]._socket for debug*/
		std::cout << "serversVec.size(): " << _serversVec.size() << std::endl;
		for (int i = 0; i < _serversVec.size() ; i++)
			std::cout << "serversVec[" << i << "]._socket : " << _serversVec[i]._socket << std::endl;
	for (size_t i = 0; i < _serversVec.size(); i++)
	{
		initEvent(event, EPOLLIN, _serversVec[i]._socket);
		ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serversVec[i]._socket, &event);
		char buffer;
		ssize_t result = read(_serversVec[i]._socket, &buffer, 0);
		if (result < 0)
		{
		    std::cout << "read error on socket " << _serversVec[i]._socket << " : " << errno << std::endl;
		}
		else
		{
		    std::cout << "socket " << _serversVec[i]._socket << " is open" << std::endl;
		}	
		if (ret < SUCCESS)
			throw EpollCtlException();
	}

	initEvent(event, EPOLLIN, STDIN_FILENO);
	ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	if (ret < SUCCESS)
		throw EpollCtlException();
	return (SUCCESS);
}

void	Webserv::handleRequest(Client &client, struct epoll_event &event)
{
	(void)event;
	std::cout << "> Parsing request" << std::endl;
	std::string	str = readFd(client.getSocket());
	client.parse(str);
	if (client.getRequest()->_statusCode != OK)
		return;
	else
		editSocket(client.getSocket(), EPOLLIN, event);
	/*
		question: est-ce qu'on a pas besoin de mettre _client.erase
		dans chaque handles(request handle / response handle)?
	*/
}

void	Webserv::handleResponse(Client &client, Request *req, struct epoll_event &event)
{
	(void)event;
	
	std::cout << "> Handling response" << std::endl;
	if (req == NULL)
		return ;
	if (req->_statusCode != OK)
		return (client.displayErrorPage(_statusCodeList.find(req->_statusCode)));
	// GENERATE RESPONSE //
	if (CGI)
	{
		if (req->getMethod() == "GET")
			getMethodCGI(client, req->getPath());
		else if (req->getMethod() == "POST")
			postMethodCGI(client, *req);
		else if (req->getMethod() == "DELETE")
			deleteMethodCGI(client, req->getPath());
		else
			return (client.displayErrorPage(_statusCodeList.find(METHOD_NOT_ALLOWED)));
	}
	else
	{
		if (req->getMethod() == "GET")
			getMethod(client, req->getPath());
		else if (req->getMethod() == "POST")
			postMethod(client, *req);
		else if (req->getMethod() == "DELETE")
			deleteMethod(client, req->getPath());
		else
			return (client.displayErrorPage(_statusCodeList.find(METHOD_NOT_ALLOWED)));
	}
	
	//->if la classe response est bien "generee"
	// editSocket(client.getSocket(), EPOLLOUT, event);
}

const char*	Webserv::EpollCreateException::what() const throw()
{
	return ("Error: Epoll_create() failed");
}

const char*	Webserv::EpollCtlException::what() const throw()
{
	return ("Error: Epoll_ctl() failed");
}

const char*	Webserv::EpollWaitException::what() const throw()
{
	return ("Error: Epoll_wait() failed");
}

const char*	Webserv::AcceptException::what() const throw()
{
	return ("Error: Accept() failed");
}