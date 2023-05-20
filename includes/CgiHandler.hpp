#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <exception>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctime>
#include <map>

//#include "Response.hpp"
//#include "Request.hpp"
#include "../includes/Utils_Cgi.hpp"

#define TIMEOUT_LIMIT 5  // limit of timeout(sec)

class CgiHandler {
public:
//    CgiHandler(Response& response);
    CgiHandler();//now with test value (without other class)
    virtual ~CgiHandler();

    const std::string& getProgram() const;
    const std::string& getScriptPath() const;
    const std::map<std::string, std::string>& GetEnv() const;
    char** GetEnvAsCstrArray() const;
    void FreeEnvCstrArray(char** env) const;


    bool getCgiOutput(std::string& output);

    class Error : public virtual std::exception {
    public:
        Error(const char* msg) : _msg(msg) {}

        const char* what() const throw() {
            return _msg;
        }

    private:
        const char* _msg;
    };

private:
    void Execute();
    void Restore();
    void RedirectOutputToPipe();
    void PipeSet();
    void SetupParentIO();
    bool WaitforChild(int pid);
    void WriteToStdin();

    void setCgiEnvironment();
    void TestEnv();
    
//    Response *response;
//    Request *request;
    int fd_in_[2];
    int fd_out_[2];
    int _in;
    int _out;
    const std::string _scriptPath;
    const std::string _program;
    std::string _request_body;
    std::map<std::string, std::string> _env;
};

#endif