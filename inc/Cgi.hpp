/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/08 14:47:56 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 00:41:13 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "Utils.hpp"
#include "Request.hpp"
#include "Location.hpp"

class Request;

class Cgi {

	public:

        // Constructors and Destructor
		Cgi();
		Cgi(std::string path);
		~Cgi();
		Cgi(Cgi const &other);
		Cgi &operator=(Cgi const &rhs);

        // Methods
		void		initGenericCgiEnv(Request& req, const std::vector<Location>::iterator it_loc);
		void		initCgiEnv(Request& req, const std::vector<Location>::iterator it_loc);
		void		clearCgiEnv();
		std::string	extractCgiEnv(std::string &path);
		void		cgiExec(short &error_code);
		int			findStart(const std::string path, const std::string sep);
		std::string	getPathInfo(std::string& path, std::vector<std::string> extensions);

        // Getters and Setters
		void setCgiPid(pid_t cgi_pid);
		void setCgiPath(const std::string &cgi_path);
		const std::map<std::string, std::string> &getEnv() const;
		const pid_t &getCgiPid() const;
		const std::string &getCgiPath() const;

		int	pipe_in[2];
		int	pipe_out[2];

	private:
		std::map<std::string, std::string>	_env;
		char**								_ch_env;
		char**								_argv;
		int									_exit_status;
		std::string							_cgi_path;
		pid_t								_cgi_pid;
};

#endif
