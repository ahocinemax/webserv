#include "includes/CgiHandler.hpp"
#include <iostream>

int main()
{
    CgiHandler cgiHandler;

    std::string output;
    if (cgiHandler.getCgiOutput(output)) {
        std::cout << output << std::endl;
        std::cout << "program executed" <<std::endl;
    } else {
        std::cerr << "Failed to execute CGI script." << std::endl;
    }
    return 0;
}
