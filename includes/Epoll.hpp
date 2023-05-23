#ifndef EPOLL_HPP
# define EPOLL_HPP

# include "Utils.hpp"
# include <sys/epoll.h>

# define MAX_EPOLL_EVENTS 1000

class Epoll
{
    public:
        Epoll(void);
        ~Epoll(void);

        void		addFd(int fd);
        void		deleteFd(int fd);
        void		modifyFd(int fd, uint32_t events);
        int			wait(int timeout);
        void        initEpoll(void);

        struct epoll_event	_events[MAX_EPOLL_EVENTS];

    private:
        int		_epollFd;
};

#endif