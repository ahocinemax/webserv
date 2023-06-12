#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctime>
#include <map>

#include "../includes/Response.hpp"
#include "../includes/Request.hpp"
#include "../includes/Utils_Cgi.hpp"
#include "../includes/Utils.hpp"

#define TIMEOUT_LIMIT 5  // limit of timeout(sec)
class   Request;

class CgiHandler
{
    public:
        CgiHandler(Request& request);
        CgiHandler(Response& response);
        CgiHandler();//now with test value (without other class)
        virtual ~CgiHandler();

        const std::string&                          getProgram() const;
        const std::string&                          getScriptPath() const;
        const std::map<std::string, std::string>&   GetEnv() const;
        void                                        setEnv(const std::string &key, const std::string &val);
        char**                                      GetEnvAsCstrArray() const;
        void                                        FreeEnvCstrArray(char** env) const;
        bool                                        getCgiOutput(std::string& output);
        void                                        initCgiEnvironment();

        class CgiError : public virtual std::exception 
        {
            public:
                CgiError(const char* msg) : _msg(msg) {}

                const char* what() const throw() {
                    return (_msg);
                }

            private:
                const char* _msg;
        };

    private:
        void    Execute();
        void    Restore();
        void    RedirectOutputToPipe();
        void    PipeSet();
        void    SetupParentIO();
        bool    WaitforChild(int pid);
        void    WriteToStdin();
        void    TestEnv();
    
        Response                            *_response;
        Request                             *_request;
        int                                 fd_in[2];
        int                                 fd_out[2];
        std::string                         _request_body;
        int                                 _in;
        int                                 _out;
        std::map<std::string, std::string>  _env;
        const std::string                   _scriptPath;
        const std::string                   _program;
};

#endif