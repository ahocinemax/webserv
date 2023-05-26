#include "Webserv.hpp"

void	Webserv::initEvent(struct epoll_event &event, uint32_t flag, int fd)
{
	memset(&event, 0, sizeof(event));
	event.events = flag;
	event.data.fd = fd;
}

void	Webserv::initConnection(int index)
{
	struct epoll_event	event;
	struct sockaddr		clientAddr;
	socklen_t			clientAddrLen = sizeof(clientAddr);
	int					clientSock;

	if ((clientSock = accept(index, &clientAddr, &clientAddrLen)) < SUCCESS && !(errno == EAGAIN || errno == EWOULDBLOCK))
		throw AcceptException();
	if (_clients[index].setSocket(clientSock) == FAILED)
		;

	initEvent(event, EPOLLIN | EPOLLET, clientSock);
	std::cout << YELLOW << "[Accept]" << RESET << " connection on socket " + to_string(index) + " at " + _clients[index]._server->_ipAddress + ":" + _clients[index]._server->_port << std::endl;
	std::cout << PURPLE << std::setw(52) << "socket " + to_string(clientSock) + " created to communicate" << RESET << std::endl;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, clientSock, &event) < SUCCESS)
		throw EpollCtlException();
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

int	Webserv::routine(void)
{
	struct epoll_event	events[MAX_EPOLL_EVENTS];
	int					nbEvents = 0;
	int					index = 0;

	if ((nbEvents = epoll_wait(_epollFd, events, MAX_EPOLL_EVENTS, -1)) < SUCCESS)
		throw EpollWaitException();

	for (int i = 0; i < nbEvents; i++)
	{
		if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
			return (close(events[i].data.fd), SUCCESS);
		else if ((index = findClientIndex(events[i].data.fd)) >= SUCCESS) // Le client n'existe pas encore
			initConnection(index);
		else if (events[i].data.fd == STDIN_FILENO)
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return (FAILED);
		}
		else
		{
			handleRequest(_clients[i], events[i]);
		}
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
		initEvent(event, EPOLLIN, _serversVec[i]._socket);
		ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, _serversVec[i]._socket, &event);
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

	// Lire la requête
	// Parser la requête
	// Créer la réponse
	// Envoyer la réponse
	// Fermer le client (keep-alive ?)
	std::string	str;
	str = readfd(client.getfd());
	int a = client.parse(str);
	if (a == INCOMPLETE)
		return;
	else if 
		//gestion pour creation socket? je ne suis pas sure pour epoll()

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