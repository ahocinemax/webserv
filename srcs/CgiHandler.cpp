/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtsuji <mtsuji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 11:45:24 by mtsuji            #+#    #+#             */
/*   Updated: 2023/06/19 11:45:29 by mtsuji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"

CgiHandler::CgiHandler(Request &request) : _response(0),
										   _request(request),
										   _request_body(request.getBody()),
										   _in(-1),
										   _out(-1),
										   _scriptPath(request.getRoot()),
										   _program("")
{
	size_t last_dot = _scriptPath.find_last_of(".");
	if (last_dot != std::string::npos)
	    _pos_exec = _scriptPath.substr(last_dot);
	initCgiEnvironment();
    if (!_pos_exec.empty())
	{
        setProgram(_pos_exec);
		_program = getProgram();
	}
	if (!AccessiblePath(_scriptPath))
	{
		_request._statusCode = NOT_FOUND;
		std::cerr << "error path" << std::endl;
		return;
	}
	SigpipeSet(0);
}

CgiHandler::~CgiHandler()
{
	Restore();
	SigpipeSet(1);
}

void CgiHandler::initCgiEnvironment()
{
	_env["AUTH_TYPE"] = "";
	_env["CONTENT_TYPE"] = _request.getHeader("content-type");
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["PATH_TRANSLATED"] = "";
	_env["QUERY_STRING"] = _request.getQuery();
	_env["SERVER_PROTOCOL"] = _request.getProtocolHTTP();
	_env["REQUEST_METHOD"] = _request.getMethod();
	_env["SCRIPT_NAME"] = _request.getPath();
	_env["SERVER_PORT"] = to_string(_request.getPort());
	_env["REMOTE_IDENT"] = _request.getHeader("autorization");
	_env["REMOTE_ADDR"] = _request.getHost();
	_env["SCRIPT_FILENAME"] = get_cgipath(); // traduire filename par path -> coder au 2/06
	_env["SERVER_NAME"] = "webserv";
	_env["SERVER_SOFTWARE"] = "webserv";
	_env["PATH_INFO"] = "";
	_env["CONTENT_LENGTH"] = to_string(_request.getSize());

	_env["HTTP_ACCEPT"] = _request.getHeader("accept");
	_env["HTTP_ACCEPT_LANGUAGE"] = _request.getHeader("accept-language");
	_env["HTTP_USER_AGENT"]= _request.getHeader("user-agent");
	_env["HTTP_COOKIE"] = _request.getHeader("cookie");
	_env["HTTP_REFERER"] = _request.getHeader("referer");
	_env["REDIRECT_STATUS"] = to_string(_request._statusCode);
}

void CgiHandler::setEnv(const std::string &key, const std::string &val)
{
	_env[key] = val;
}

void CgiHandler::setProgram(const std::string& program)
{
    if (program == ".php")
        _program = PHP_EXEC;
    else if (program == ".py")
        _program = PYTHON_EXEC;
    else
    {
        std::cerr << "Unsupported extension: "  << std::endl;
        _program = "";
    }
}

const std::map<std::string, std::string> &CgiHandler::getEnv() const
{
	return _env;
}

const std::string &CgiHandler::getScriptPath() const
{
	return (_scriptPath);
}

const std::string &CgiHandler::getProgram() const
{
	return (_program);
}


bool CgiHandler::getCgiOutput(std::string &output)
{
	PipeSet();
	int pid = fork();
	if (pid < 0)
	{
		std::cerr << "cgihandler:getCgiOutput (fork) error" << std::endl;
		return (false);
	}
	else if (pid == 0)
	{
		/*child*/
		usleep(900);
		Execute();
		exit(EXIT_FAILURE);
		//return true;
	}
	else
	{
		/*parents*/

		if (WaitforChild(pid))
		{
			output = readFd(fd_out[0]);
			if (containHeader(output))
				removeHeader(output);
			close(fd_out[0]);
			return true;
		}
		else
		{
			close(fd_out[0]);
			return false;
		}
	}
}

std::string CgiHandler::get_cgipath() const
{
	std::string translation(std::string(PWD));
	std::string relativePath = _scriptPath;

	if (relativePath[0] == '.')
		relativePath.erase(0, 1);
	else if (relativePath[0] != '/')
		relativePath.insert(0, "/");
	translation += relativePath;
	return (translation);
}

char **CgiHandler::getEnvAsCstrArray() const
{
	char **env = new char *[this->_env.size() + 1];
	if (!env)
		return (0);
	int j = 0;
	for (StringMap::const_iterator i = this->_env.begin(); i != this->_env.end(); i++)
	{
		std::string element = i->first + "=" + i->second;
		env[j] = new char[element.size() + 1];
		if (!env[j])
			return (freeEnvCstrArray(env), (char **)0);
		strcpy(env[j], element.c_str());
		j++;
	}
	env[j] = NULL;
	return (env);
}

void CgiHandler::freeEnvCstrArray(char **env) const
{
	for (int i = 0; env[i] != NULL; i++)
	{
		delete[] env[i];
	}
	delete[] env;
}

void CgiHandler::Execute()
{
	char *av[] = {
		const_cast<char *>(getProgram().c_str()),
		const_cast<char *>(getScriptPath().c_str()),
		0};
	char **env = getEnvAsCstrArray();
	if (!env)
		return ;
	RedirectOutputToPipe();
	execve(av[0], av, env);
	if (errno)
		std::cerr << "execve failed with error: " << strerror(errno) << std::endl;
	if (close(fd_in[0]) || close(fd_out[1]))
	{
		std::cerr << "cgihandler: Execute (close) error" << std::endl;
		return;
	}
	freeEnvCstrArray(env);
}

void CgiHandler::Restore()
{
	if (_in == -1 && _out == -1)
		return;

	if (dup2(_in, STDIN_FILENO) < 0)
	{
		std::cerr << "cgihandler: Restore (dup2) error" << std::endl;
		return;
	}
	if (close(_in))
	{
		std::cerr << "cgihandler: Restore (close) error" << std::endl;
		return;
	}
	if (dup2(_out, STDOUT_FILENO) < 0)
	{
		std::cerr << "cgihandler: Restore (dup2) error" << std::endl;
		return;
	}
	if (close(_out))
	{
		std::cerr << "cgihandler: Restore (close) error" << std::endl;
		return;
	}
}

void CgiHandler::RedirectOutputToPipe()
{
	if (close(fd_in[1]) < 0)
	{
		std::cerr << "cgihandler: RedirectOutputToPipe (close) error" << std::endl;
		return;
	}
	if ((_in = dup(STDIN_FILENO)) < 0)
	{
		std::cerr << "cgihandler: RedirectOutputToPipe (dup) error" << std::endl;
		return;
	}
	if (dup2(fd_in[0], STDIN_FILENO) < 0)
	{
		std::cerr << "cgihandler: RedirectOutputToPipe (dup2) error" << std::endl;
		return;
	}
	if (close(fd_out[0]) < 0)
	{
		std::cerr << "cgihandler: RedirectOutputToPipe (close) error" << std::endl;
		return;
	}
	if ((_out = dup(STDOUT_FILENO)) < 0)
	{
		std::cerr << "cgihandler: RedirectOutputToPipe (dup) error" << std::endl;
		return;
	}
	if (dup2(fd_out[1], STDOUT_FILENO) < 0)
	{
		std::cerr << "cgihandler: RedirectOutputToPipe (dup2) error" << std::endl;
		return;
	}
}

void CgiHandler::PipeSet()
{
	if (pipe(fd_in) < 0)
	{
		std::cerr << "cgihandler: getCgiOutput (pipe) error" << std::endl;
		return;
	}
	if (pipe(fd_out) < 0)
	{
		std::cerr << "cgihandler: getCgiOutput (pipe) error" << std::endl;
		return;
	}
}

void CgiHandler::SetupParentIO()
{
	if (close(fd_in[0]) < 0)
	{
		std::cerr << "cgihandler: ParentIO (close) error" << std::endl;
		return;
	}
	if (close(fd_out[1]) < 0)
	{
		std::cerr << "cgihandler: ParentIO (close) error" << std::endl;
		return;
	}
}

bool CgiHandler::WaitforChild(int pid)
{
	int wstatus = 0;
	time_t start = time(0);

	WriteToStdin();
	while (true)
	{
		if (waitpid(pid, &wstatus, WNOHANG) == pid)
		{
			break;
		}
		usleep(500);
		if (std::difftime(time(0), start) >= TIMEOUT_LIMIT)
		{
			kill(pid, SIGINT);
			break;
		}
	}
	return (WIFEXITED(wstatus) && (WEXITSTATUS(wstatus) != EXIT_FAILURE));
}

void CgiHandler::WriteToStdin()
{
	SetupParentIO();
	if (write(fd_in[1], _request_body.c_str(), _request_body.size()) < 0)
	{
		std::cerr << "cgihandler: WriteToStdin (write) error" << std::endl;
		return ;
	}
	if (close(fd_in[1]) < 0)
	{
		std::cerr << "cgihandler: WriteToStdin (close) error" << std::endl;
		return ;
	}
}

bool CgiHandler::containHeader(std::string &output)
{
	return (output.find("\r\n\r\n") == std::string::npos) ? false : true;
}

void CgiHandler::removeHeader(std::string &output)
{
	output = output.substr(output.find("\r\n\r\n") + 4);
}