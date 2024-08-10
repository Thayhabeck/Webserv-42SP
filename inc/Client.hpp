/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 22:44:33 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/10 00:41:52 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Utils.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"

/** Armazena todas as informações relacionadas ao cliente, como socket, endereço IP, request e response. *
 * Cada Cliente também armazena o servidor ao qual está conectado. **/
class Client
{
    public:

        // Construtores e Destrutor
        Client();
        Client(const Client &other);
        Client(Server &);
		Client &operator=(const Client & rhs);
        ~Client();

        // Métodos
        void                        buildResponse();
        void                        updateTimeLastMessage();
        void                        clearClient();

        // Getters e Setters
        const int                   &getSocket() const;
        const struct sockaddr_in    &getAddress() const;
        const Request               &getRequest() const;
        const time_t                &getTimeLastMessage() const;
        void                        setSocket(int &);
        void                        setAddress(sockaddr_in &);
        void                        setServer(Server &);

        Response            response;
        Request             request;
        Server              server;
        
    private:
    
        int                 _cli_socket;
        struct sockaddr_in  _cli_address;
        time_t              _cli_last_msg;
};


#endif