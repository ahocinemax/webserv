#include "../includes/Utils_Cgi.hpp"

void	SigPipeIgnore(int sig) { (void) sig; }

void	SigpipeSet(int state)
{
	void	(*oldHandler_SigPipe)(int) = 0;

	if (state == 0) {
		oldHandler_SigPipe = signal(SIGPIPE, SigPipeIgnore);
	} else if (state == 1) {
		signal(SIGPIPE, oldHandler_SigPipe);
	}
}

bool	AccessiblePath(const std::string& path)
{
	int	ret;

	ret = access(path.c_str(), F_OK); 
	return (!ret);
}