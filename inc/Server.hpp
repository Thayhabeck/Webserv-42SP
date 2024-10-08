/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/07 22:21:46 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/11 14:39:12 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Utils.hpp"
#include "Location.hpp"

static std::string serverParams[] =
{
	"server_name",
	"listen",
	"root",
	"index",
	"allow_methods",
	"client_body_buffer_size"
};

class Location;

class Server
{
	public:
		Server();
		~Server();
		Server(const Server &other);
		Server &operator=(const Server &rhs);

		void		initErrorPages(void);
		void		setupServer();
		int			getServerFd();
		bool		checkErrorPages();
		bool		checkLocations() const;
		bool		checkHost(std::string host) const;
		static void	checkToken(std::string &parametr);

		void	setServerName(std::string server_name);
		void	setHost(std::string parametr);
		void	setRoot(std::string root);
		void	setFd(int);
		void	setPort(std::string parametr);
		void	setClientMaxBodySize(std::string parametr);
		void	setErrorPages(std::vector<std::string> &parametr);
		void	setIndex(std::string index);
		void	setLocation(std::string nameLocation, std::vector<std::string> parametr);
		void	setAutoindex(std::string autoindex);
		const std::string						&getServerName();
		const uint16_t							&getPort();
		const in_addr_t							&getHost();
		const size_t							&getClientMaxBodySize();
		const std::vector<Location>				&getLocations();
		const std::string						&getRoot();
		const std::map<short, std::string>		&getErrorPages();
		const std::string						&getIndex();
		const bool								&getAutoindex();
		const std::string						&getPathErrorPage(short key);
		const std::vector<Location>::iterator	getLocationKey(std::string key);

		class ServerConfigErrorException : public std::exception
		{
			private:
				std::string _message;

			public:
				ServerConfigErrorException(std::string message) throw()
				{
					_message = RED "Server Config Error: " RESET + message;
				}
				virtual const char *what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ServerConfigErrorException() throw() {}
		};

	private:
		uint16_t						_port;
		in_addr_t						_host;
		std::string						_root;
		std::string						_index;
		bool							_autoindex;
		std::vector<Location>			_locations;
		std::string						_server_name;
		std::map<short, std::string>	_error_pages;
		unsigned long					_client_max_body_size;

		int								_listen_fd;
		struct sockaddr_in				_server_address;
};

#endif
