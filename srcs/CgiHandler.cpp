#include "../includes/CgiHandler.hpp"

CGIHandler::CGIHandler() {}

CGIHandler::~CGIHandler() {}

std::string CGIHandler::executeCgiScript(const std::string& scriptPath, const std::string& requestBody) {
//exection
}



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