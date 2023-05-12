#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include <string>

class CGIHandler {
public:
    CGIHandler();
    ~CGIHandler();

    std::string executeCgiScript(const std::string& scriptPath, const std::string& requestBody);
};

#endif
