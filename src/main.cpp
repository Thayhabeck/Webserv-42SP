/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 23:52:58 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/17 02:01:22 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Utils.hpp"
#include "../inc/ServerConf.hpp"
#include "../inc/Webserv.hpp"

ServerConf	server_config;
Webserv		webserv;

void	_server_interrupt(int sig)
{
	std::cout << "\nServer Interrupted\nSignal: " << (sig + 128) << "\n";;
	webserv.close_servers();
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) 
{
	if (argc == 1 || argc == 2) {
		try 
		{
			struct sigaction	interrupt_handler;
			interrupt_handler.sa_handler = _server_interrupt;
			sigemptyset(&interrupt_handler.sa_mask);
			interrupt_handler.sa_flags = 0;
			sigaction(SIGINT, &interrupt_handler, 0);

			server_config.createServerPool(argc == 1 ? "./conf/default.conf" : argv[1]);
			webserv.initServers(server_config.getServerPool());
			webserv.runServers();
		}
		catch (std::exception &e)
		{
			std::cerr << e.what() << std::endl;
			return (1);
		}
	}
	else 
	{
		std::cerr << "Error: wrong arguments" << std::endl;
		return (1);
	}
	return (0);
}
