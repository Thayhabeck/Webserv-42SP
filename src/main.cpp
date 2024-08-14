/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 23:52:58 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 22:43:29 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Utils.hpp"
#include "../inc/ServerConf.hpp"
#include "../inc/Webserv.hpp"

void sigpipeHandle(int sig) { if(sig) {}}

int main(int argc, char **argv) 
{
	if (argc == 1 || argc == 2) {
		try 
		{
			signal(SIGPIPE, sigpipeHandle);
			ServerConf	server_config;
			Webserv		webserv;
			std::string conf_file = (argc == 1 ? "./conf/default.conf" : argv[1]);
			server_config.createServerPool(conf_file);
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
