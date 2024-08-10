/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 22:57:28 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 00:45:07 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
# define WEBSERV_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Client.hpp"

/**
 * - Executa os servidores com as configurações constantes arquivo de configuração
 * - Recebe e envia requisições e respostas
 */
class Webserv
{
    public:
    
        // Construtores e Destrutor           
        Webserv();
        ~Webserv();

        // Métodos
        void    initServers(std::vector<Server>);
        void    runServers();
        
    private:

        // Métodos privados
        void    openCliConnection(Server &);
        void    closeCliConnection(const int);
        void    assignServerToClient(Client &);
        void    initializeSets();
        void    addToSet(const int , fd_set &);
        void    removeFromSet(const int , fd_set &);
        void    checkTimeout();
        void    readRequest(const int &, Client &);
        void    handleRequestBody(Client &);
        void    sendResponse(const int &, Client &);
        void    sendCgiBody(Client &, Cgi &);
        void    readCgiResponse(Client &, Cgi &);

        // Atributos
        std::vector<Server> _servers;
        std::map<int, Server> _servers_map;
        std::map<int, Client> _clients_map;
        fd_set     _recv_fd_pool;
        fd_set     _write_fd_pool;
        int        _biggest_fd;
};


#endif