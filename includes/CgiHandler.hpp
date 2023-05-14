/*
#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>

class CGIHandler {
public:
    CGIHandler();
    ~CGIHandler();

    std::string executeCgiScript(const std::string& scriptPath, const std::string& requestBody);
};
*/

//#endif


#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <exception>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctime>

#include "Response.hpp"
#include "Request.hpp"
#include "Utils_Cgi.hpp"

#define TIMEOUT_LIMIT 5  // limit of timeout(sec)

class CgiHandler {
public:
    CgiHandler(Response& response);
    virtual ~CgiHandler();

    const std::string& getProgram() const;
    const std::string& getScriptPath() const;
    const std::map<std::string, std::string>& getEnv() const;
    char** GetEnvAsCstrArray() const;
    void FreeEnvCstrArray(char** env) const;


    bool getCgiOutput(std::string& output);

    class RuntimeError : public virtual std::exception {
    public:
        RuntimeError(const char* msg) : _msg(msg) {}

        const char* what() const throw() {
            return _msg;
        }

    private:
        const char* _msg;
    };

private:
    void _execute();
    void _restore();
    void _redirectToPipe();
    void _setupPipe();
    void _setupParentIo();
    bool _waitForChild(int pid);
    void _writeToStdin();

    void _setCgiEnvironment();
    //getter for mime ? 
    
    Response *response;
    Request *request;
    int fd_in_[2];
    int fd_out_[2];
    int in_;
    int out_;
    const std::string script_path_;
    std::string request_body_;
    std::map<std::string, std::string> _env;
};

#endif