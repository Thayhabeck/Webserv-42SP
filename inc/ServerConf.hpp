/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 20:24:13 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 22:44:54 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONF_HPP
#define SERVERCONF_HPP

#include "Utils.hpp"
#include "Server.hpp"

class Server;

class ServerConf {
	private:
		std::vector<std::string>	_server_config;
		std::vector<Server>			_servers;
		size_t						_nb_server;

	public:
		ServerConf();
		~ServerConf();

		int					createServerPool(const std::string &config_file);
		void				rmComments(std::string &content);
		void				rmSpaces(std::string &content);
		void				createServer(std::string &config, Server &server);
		void				checkCompareServersParams();
		void				splitServerConfStr(std::string &content);
		int					strCompare(std::string str1, std::string str2, size_t pos);
		size_t				getServerStartPosition(size_t start, std::string &content);
		size_t				getServerEndPosition(size_t start, std::string &content);
		std::vector<Server>	getServerPool();

		public:
		class ServerConfigErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ServerConfigErrorException(std::string message) throw()
				{
					_message = RED "Server Config Error: " RESET + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ServerConfigErrorException() throw() {}
		};
};

#endif
