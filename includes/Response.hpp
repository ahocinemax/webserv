#ifndef RESPONSE_HPP
# define RESPONSE_HPP

#include "Utils.hpp"

class Response
{
    public:
        Response(void);
        Response(std::string statutCode);
        ~Response();

        std::string    makeHeader(void);
};

#endif