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

	initEvent(event, EPOLLIN | EPOLLET, client.getSocket());
	std::cout << YELLOW << "[Accept]" << RESET << " connection on socket " + to_string(client._server->_ipAddress) + " at " + client._server->_ipAddress + ":" + client._server->_port << std::endl;
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
		if ((index = findClientIndex(events[i].data.fd)) == FAILED) // si le client n'existe pas encore
			index = initConnection(events[i].data.fd);
		else if (events[i].data.fd == STDIN_FILENO)
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			return (FAILED);
		}

		handleRequest(_clients[index], events[i]);

		if (false) // si le client est déconnecté, il faut l'enlever de la liste
			_clients.erase(_clients.begin() + index);
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
		std::cout << "serversVec.size()" << _serversVec.size() << std::endl;
		std::cout << "serversVec[0]._socket : " << _serversVec[0]._socket << std::endl;
		std::cout << "serversVec[1]._socket : " << _serversVec[1]._socket << std::endl;
		std::cout << "serversVec[2]._socket : " << _serversVec[2]._socket << std::endl;
		std::cout << "serversVec[3]._socket : " << _serversVec[3]._socket << std::endl;
		std::cout << "serversVec[4]._socket : " << _serversVec[4]._socket << std::endl;
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
		{
			std::cout << YELLOW "Connect epoll " RESET << _epollFd << YELLOW " to sockets: " RESET << _serversVec[i]._socket << std::endl;
			throw EpollCtlException();
		}
		std::cout << YELLOW "Connect epoll " RESET << _epollFd << YELLOW " to sockets: " RESET << _serversVec[i]._socket << std::endl;
	}

	initEvent(event, EPOLLIN, STDIN_FILENO);
	ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
	if (ret < SUCCESS)
	{
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
	//char	buf[BUFFER_SIZE + 1];

	//int lu = read(client.getSocket(), buf, BUFFER_SIZE);
	///std::string	str(buf);
	//int a = client.parse(str);
	std::string	str = readFd(client.getSocket());
	std::cout << "readFd returned:\n" << BLUE << str << WHITE << std::endl;
	int a = client.parse(str);
	if (a == INCOMPLETE)
		return;
	else if (false)
		; // gestion pour creation socket? je ne suis pas sure pour epoll()

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