#include "../includes/CgiHandler.hpp"


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
    //_request_body(response.GetRequest()->GetBody()),
    _in(-1),
    _out(-1),
    //_scriptPath(response.GetBuiltPath()),
    //_program(reponse.GetCgiProgram()),
    //for test
    _scriptPath("/mnt/nfs/homes/mtsuji/Documents/level5/webserv/ahocine/document"),
    _program("/usr/local/bin/perl"),
    _env(GetEnv())
    {
        if (!AccessiblePath(_scriptPath)) {
		    response.getStatusCode() = "400";
            return;
        }
        SigpipeSet(0);
    }

//CgiHandler::CgiHandler() : 
//    //_response(response),
//    _request(0),
//    //_scriptPath(response.GetBuiltPath()),
//    _request_body(""),
//    _in(-1),
//    _out(-1),
//    //_program(reponse.GetCgiProgram()),
//    /*for test*/
//    _env(GetEnv()),
//    _scriptPath("/mnt/nfs/homes/mtsuji/Documents/level5/webserv/ahocine/document/tohoho.cgi"),//cluter
//    //_scriptPath("/home/tj/Documents/42/webserv/ahocine/document/tohoho.cgi"),//home
//    _program("/usr/bin/perl")
//    {
//        if (!AccessiblePath(_scriptPath))
//        {
//		    _response->getStatusCode() = "400";
//            return;
//        }
//        SigpipeSet(0);
//    }
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
    if (pid < 0)
    {
    	std::cerr << "cgihandler:getCgiOutput (fork) error" << std::endl;
        return (false);
    } else if (pid == 0) {
        /*child*/
        usleep(900);
        Execute();
        return true;
    } else {
        /*parents*/
        if (WaitforChild(pid)) {
            output = readFd(_fd_out[0]);
            close(_fd_out[0]);
            return true;
        } else {
            close(_fd_out[0]);
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
    setCgiEnvironment();
    //TestEnv();
    char **env = GetEnvAsCstrArray();
    RedirectOutputToPipe();
    execve(av[0], av, env);
    if (close(_fd_in[0]))
    {
    	std::cerr << "cgihandler: Execute (close) error" << std::endl;
        return ;
    }
    if (close(_fd_out[1]))
    {
    	std::cerr << "cgihandler: Execute (close) error" << std::endl;
        return ;
    }
    for (size_t i = 0; env[i]; i++)
        delete[] env[i];
    delete[] env;
}

void CgiHandler::Restore()
{
    if (_in == -1 && _out == -1)
        return;

    if (dup2(_in, STDIN_FILENO) < 0)
    {
    	std::cerr << "cgihandler: Restore (dup2) error" << std::endl;
        return ;
    }
    if (close(_in))
    {
    	std::cerr << "cgihandler: Restore (close) error" << std::endl;
        return ;
    }
    if (dup2(_out, STDOUT_FILENO) < 0)
    {
    	std::cerr << "cgihandler: Restore (dup2) error" << std::endl;
        return ;
    }
    if (close(_out))
    {
    	std::cerr << "cgihandler: Restore (close) error" << std::endl;
        return ;
    }
}

void CgiHandler::RedirectOutputToPipe()
{
    if (close(_fd_in[1]) < 0)
    {
    	std::cerr << "cgihandler: RedirectOutputToPipe (close) error" << std::endl;
        return ;
    }
    if ((_in = dup(STDIN_FILENO)) < 0)
    {
    	std::cerr << "cgihandler: RedirectOutputToPipe (dup) error" << std::endl;
        return ;
    }
    if (dup2(_fd_in[0], STDIN_FILENO) < 0) {
    	std::cerr << "cgihandler: RedirectOutputToPipe (dup2) error" << std::endl;
        return ;
    }
    if (close(_fd_out[0]) < 0) {
    	std::cerr << "cgihandler: RedirectOutputToPipe (close) error" << std::endl;
        return ;
    }
    if ((_out = dup(STDOUT_FILENO)) < 0) {
    	std::cerr << "cgihandler: RedirectOutputToPipe (dup) error" << std::endl;
        return ;
    }
    if (dup2(_fd_out[1], STDOUT_FILENO) < 0) {
    	std::cerr << "cgihandler: RedirectOutputToPipe (dup2) error" << std::endl;
        return ;
    }
}

void CgiHandler::PipeSet()
{
    if (pipe(_fd_in) < 0)
    {
    	std::cerr << "cgihandler: getCgiOutput (pipe) error" << std::endl;
        return ;
    }
    if (pipe(_fd_out) < 0)
    {
    	std::cerr << "cgihandler: getCgiOutput (pipe) error" << std::endl;
        return ;
    }
}

void CgiHandler::SetupParentIO()
{
    if (close(_fd_in[0]) < 0)
    {
    	std::cerr << "cgihandler: ParentIO (close) error" << std::endl;
        return ;
    }
    if (close(_fd_out[1]) < 0)
    {
    	std::cerr << "cgihandler: ParentIO (close) error" << std::endl;
        return ;
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
    if (write(_fd_in[1], _request_body.c_str(), _request_body.size()) < 0) {
    	std::cerr << "cgihandler: WriteToStdin (write) error" << std::endl;
        return ;
    }
    if (close(_fd_in[1]) < 0) {
    	std::cerr << "cgihandler: WriteToStdin (close) error" << std::endl;
        return ;
    }
}

/*
    Testenv()
    test function for setCgiEnvironement();
    to put test value in environement variable for CGI

void CgiHandler::TestEnv()
{
    _env["AUTH_TYPE"] = "basic"; 
    _env["DOCUMENT_ROOT"] = "test value";
	_env["SERVER_PROTOCOL"] = "HTTP/1.0";
    _env["CONTENT_TYPE"] = "test value";
	_env["REQUEST_METHOD"] = "GET";
	_env["SCRIPT_NAME"] = "test value";
    _env["CONTENT_LENGTH"] = "100";
	_env["SERVER_PORT"] = "8080";
    _env["PATH_TRANSLATED"] = "/mnt/nfs/homes/mtsuji/Documents/level5/webserv/ahocine/ubuntu_cgi_tester";
	_env["PATH_INFO"] = "test value";
    _env["REMOTE_IDENT"] = "test value";
	_env["REMOTE_ADDR"] = "localhost";
    _env["SCRIPT_FILENAME"] = "tohoho.pl";
	_env["QUERY_STRING"] = "test value";
    _env["REDIRECT_STATUS"] = "test status_code";

	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_NAME"] = "webserv";
	_env["SERVER_SOFTWARE"] = "webserv/1.0";

    _env["HTTP_ACCEPT"] = "accept";
    _env["HTTP_ACCEPT_LANGAGE"] = "accept-langage";
    _env["HTTP_USER_AGENT"] = "user-agent";
    _env["HTTP_COOKIE"] = "cookie";
    _env["HTTP_REFERER"] = "referer";
}
*/

void CgiHandler::setCgiEnvironment() {
	// Set up the environment variables
    _env["AUTH_TYPE"] = "test";//check rules 
    _env["DOCUMENT_ROOT"] = "test";//check rules

	_env["SERVER_PROTOCOL"] = _request->getProtocolHTTP();//request->getter for Protocol;
	_env["REQUEST_METHOD"] = _request->getMethod();//request->getter for getMethod;
	_env["SCRIPT_NAME"] = _request->getPath();//request->getter for path;
    _env["CONTENT_LENGTH"] = "test";//response->converter number to string;
	_env["SERVER_PORT"] = "8080";//response->converter number to string;
    _env["PATH_TRANSLATED"] = "/mnt/nfs/homes/mtsuji/Documents/level5/webserv/ahocine/ubuntu_cgi_tester";//request->getter for path;
    _env["REMOTE_IDENT"] = _request->getHeader("autorization");//request->GetHeader("Autorization");
	_env["REMOTE_ADDR"] = _request->getHost();//localhost;
    _env["SCRIPT_FILENAME"] = "tohoho.pl";//response->cgi name;
	_env["PATH_INFO"] = "argument";//response->getter for extra;
	_env["QUERY_STRING"] = "test";//response->getter for querystring;
    _env["REDIRECT_STATUS"] =  "";//response->getter for status code;

	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_NAME"] = "webserv";//response-> getter for servername;
	_env["SERVER_SOFTWARE"] = "webserv/1.0";

    _env["CONTENT_TYPE"] = "content-type";//response->makeHeader("Content-Type");
    _env["HTTP_ACCEPT"] = "accept";//response->GetHeader("accept");
    _env["HTTP_ACCEPT_LANGAGE"] = "accept-langage";//response->GetHeader("accept-langage");
    _env["HTTP_USER_AGENT"] = "user-agent";//response->makeHeader("user-agent");
    _env["HTTP_COOKIE"] = "cookie";//response->makeHeader("cookie");
    _env["HTTP_REFERER"] = "referer";//response->makeHeader("referer");
}
