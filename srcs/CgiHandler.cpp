#include "CgiHandler.hpp"

/*
  Cgi Handler 
  - Constructor
  - executeCgi Script
      -> utilisant fork pour generer un processus de fils
  - processes de fils
      -> execve pour executer le program Cgi
  - processes de parents
      -> attendre jusqu'a ce que processus de fils termine
  - lire le resultat de procesuss de fils
  - processus des parents renvoi le resultat de processus de fils 
*/


CgiHandler::CgiHandler(Response& response) : 
    //response(response),
    //_scriptPath(response.GetBuiltPath()),
    //_request_body(response.GetRequest()->GetBody()),
    _in(-1),
    _out(-1),
    //_program(reponse.GetCgiProgram()),
    _env(GetEnv())
    {
        if (!AccessiblePath(_scriptPath)) {
            throw 404;//error投げる
        }
        SigpipeSet(0);
    }

CgiHandler::~CgiHandler()
{
    Restore();
    SigpipeSet(1);
}


const std::map<std::string, std::string>& CgiHandler::GetEnv() const
{
    return _env;
}

const std::string& CgiHandler::getScriptPath() const
{
	return (_scriptPath);
}

const std::string& CgiHandler::getProgram() const
{
	return (_program);
}


bool CgiHandler::getCgiOutput(std::string& output)
{
    PipeSet();
    int pid = fork();
    if (pid < 0) {
        throw RuntimeError("cgihandler:getCgiOutput (fork) error");
    } else if (pid == 0) {
        usleep(900);
        Execute();
        return true;
    } else {
        if (WaitforChild(pid)) {
            output = readFd(fd_out_[0]);
            close(fd_out_[0]);
            return true;
        } else {
            close(fd_out_[0]);
            return false;
        }
    }
}

char** CgiHandler::GetEnvAsCstrArray() const
{
    char** env = new char*[this->_env.size() + 1];
    int j = 0;
    for (std::map<std::string, std::string>::const_iterator i = this->_env.begin(); i != this->_env.end(); i++) {
        std::string element = i->first + "=" + i->second;
        env[j] = new char[element.size() + 1];
        strcpy(env[j], element.c_str());
        j++;
    }
    env[j] = NULL;
    return env;
}

void CgiHandler::FreeEnvCstrArray(char** env) const
{
    for (int i = 0; env[i] != NULL; i++) {
        delete[] env[i];
    }
    delete[] env;
}

void CgiHandler::Execute()
{
    char* av[] = {
        const_cast<char*>(getProgram().c_str()),
        const_cast<char*>(getScriptPath().c_str()),
        0 };
    char **env = GetEnvAsCstrArray();
    setCgiEnvironment();
    RedirectOutputToPipe();
    execve(av[0], av, env);
    if (close(fd_in_[0])) {
        throw RuntimeError("cgihandler: Execute (close) error");
    }
    if (close(fd_out_[1])) {
        throw RuntimeError("cgihandler: Execute (close) error");
    }
    for (size_t i = 0; env[i]; i++)
        delete[] env[i];
    delete[] env;
}

void CgiHandler::Restore()
{
    if (_in == -1 && _out == -1) {
        return;
    }

    if (dup2(_in, STDIN_FILENO) < 0) {
        throw RuntimeError("cgihandler: Restore (dup2) error");
    }
    if (close(_in)) {
        throw RuntimeError("cgihandler: Restore (close) error");
    }
    if (dup2(_out, STDOUT_FILENO) < 0) {
        throw RuntimeError("cgihandler: Restore (dup2) error");
    }
    if (close(_out)) {
        throw RuntimeError("cgihandler: Restore (close) error");
    }
}

void CgiHandler::RedirectOutputToPipe()
{
    if (close(fd_in_[1]) < 0) {
        throw RuntimeError("cgihandler: RedirectOutputToPipe (close) error");
    }
    if ((_in = dup(STDIN_FILENO)) < 0) {
        throw RuntimeError("cgihandler: RedirectOutputToPipe (dup) error");
    }
    if (dup2(fd_in_[0], STDIN_FILENO) < 0) {
        throw RuntimeError("cgihandler: RedirectOutputToPipe (dup2) error");
    }
    if (close(fd_out_[0]) < 0) {
        throw RuntimeError("cgihandler: RedirectOutputToPipe (close) error");
    }
    if ((_out = dup(STDOUT_FILENO)) < 0) {
        throw RuntimeError("cgihandler: RedirectOutputToPipe (dup) error");
    }
    if (dup2(fd_out_[1], STDOUT_FILENO) < 0) {
        throw RuntimeError("cgihandler: RedirectOutputToPipe (dup2) error");
    }
}

void CgiHandler::PipeSet()
{
    if (pipe(fd_in_) < 0) {
        throw RuntimeError("cgihandler: getCgiOutput (pipe) error");
    }
    if (pipe(fd_out_) < 0) {
        throw RuntimeError("cgihandler: getCgiOutput (pipe) error");
    }
}

void CgiHandler::SetupParentIO()
{
    if (close(fd_in_[0]) < 0) {
        throw 500;
    }
    if (close(fd_out_[1]) < 0) {
        throw 500;
    }
}

bool CgiHandler::WaitforChild(int pid)
{
    int wstatus;
    time_t start = std::time(0);

    WriteToStdin();
    while (true) {
        if (waitpid(pid, &wstatus, WNOHANG) == pid) {
            break;
        }
        usleep(50);
        if (std::difftime(time(0), start) >= TIMEOUT_LIMIT) {
            kill(pid, SIGINT);
            break;
        }
    }
    if (WIFEXITED(wstatus) && (WEXITSTATUS(wstatus) != EXIT_FAILURE))
            return true;
    else
        return false;
}

void CgiHandler::WriteToStdin()
{
    SetupParentIO();
    if (write(fd_in_[1], _request_body.c_str(), _request_body.size()) < 0) {
        throw RuntimeError("cgihandler: WriteToStdin (write) error");
    }
    if (close(fd_in_[1]) < 0) {
        throw RuntimeError("cgihandler: WriteToStdin (close) error");
    }
}

void CgiHandler::setCgiEnvironment() {
	// ...

	// Set up the environment variables
    //_env["AUTH_TYPE"] = //check rules 
    //_env["DOCUMENT_ROOT"] = //check rules

	//_env["SERVER_PROTOCOL"] = request->getter for Protocol;
    _env["CONTENT_TYPE"] = request->GetHeader("Content-Type");
	//_env["REQUEST_METHOD"] = request->getter for getMethod;
	//_env["SCRIPT_NAME"] = request->getter for path;
    //_env["CONTENT_LENGH"] = response->converter number to string;
	//_env["SERVER_PORT"] = response->converter number to string;
   // _env["PATH_TRANSLATED"] = request->getter for path;
    _env["REMOTE_IDENT"] = request->GetHeader("Autorization");
	//_env["REMOTE_ADDR"] = //localhost;
    //_env["SCRIPT_FILENAME"] = response->cgi name;
	//_env["PATH_INFO"] = response->getter for extra;
	//_env["QUERY_STRING"] = response->getter for querystring;
    //_env["REDIRECT_STATUS"] =  response->getter for status code;

	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	//_env["SERVER_NAME"] = response-> getter for servername;
	_env["SERVER_SOFTWARE"] = "webserv/1.0";

    _env["HTTP_ACCEPT"] = request->GetHeader("accept");
    _env["HTTP_ACCEPT_LANGAGE"] = request->GetHeader("accept-langage");
    _env["HTTP_USER_AGENT"] = request->GetHeader("user-agent");
    _env["HTTP_COOKIE"] = request->GetHeader("coockie");
    _env["HTTP_REFERER"] = request->GetHeader("referer");
}


/*
    memo: Is "mime" type required in CGI? - I don't think so
    16/05 need to add:
    c
*/
