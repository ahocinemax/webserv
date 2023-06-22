/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtsuji <mtsuji@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 11:45:12 by mtsuji            #+#    #+#             */
/*   Updated: 2023/06/19 11:45:15 by mtsuji           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
#include "../includes/Utils.hpp"

#define TIMEOUT_LIMIT 5
class   Request;

class CgiHandler
{
	public:
		time_t										_timer;
		int											_pid;

		CgiHandler(Request& request);
		virtual ~CgiHandler();
		CgiHandler &operator=(const CgiHandler &other);

		void                                        initCgiEnvironment();
		void                                        setEnv(const std::string &key, const std::string &val);
		const std::map<std::string, std::string>&   getEnv() const;
		const std::string&                          getScriptPath() const;
		void                                       setProgram(const std::string& program);
		bool                                        getCgiOutput(std::string& output);
		std::string                                 get_cgipath() const;
		char**                                      getEnvAsCstrArray() const;
		void                                        freeEnvCstrArray(char** env) const;
		const std::string&                          getProgram() const;

	private:
		void                                Execute(void);
		void                                Restore(void);
		void                                RedirectOutputToPipe(void);
		void                                PipeSet(void);
		void                                SetupParentIO(void);
		bool                                WaitforChild(int pid);
		bool                                WriteToStdin(void);
		bool                                containHeader(std::string& output);
		void                                removeHeader(std::string& output);

		Response							*_response;
		Request								_request;
		int									fd_in[2];
		int                                 fd_out[2];
		std::string                         _request_body;
		int									_in;
		int									_out;
		std::map<std::string, std::string>	_env;
		std::string							_scriptPath;
		std::string							_program;
		std::string							_pos_exec;
};

#endif
