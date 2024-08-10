/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 22:50:38 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/09 23:34:54 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Client.hpp"

Client::Client() : _cli_last_msg(time(NULL)) {}

Client::Client(Server &server) : _cli_last_msg(time(NULL))
{
    setServer(server);
    request.setMaxBodySize(server.getClientMaxBodySize());
}

Client::~Client() {}

Client::Client(const Client &other)
{
	if (this != &other)
	{
		this->_cli_socket = other._cli_socket;
		this->_cli_address = other._cli_address;
		this->request = other.request;
		this->response = other.response;
		this->server = other.server;
		this->_cli_last_msg = other._cli_last_msg;

	}
	return ;
}

Client &Client::operator=(const Client & rhs)
{
	if (this != &rhs)
	{
		this->_cli_socket = rhs._cli_socket;
		this->_cli_address = rhs._cli_address;
		this->request = rhs.request;
		this->response = rhs.response;
		this->server = rhs.server;
		this->_cli_last_msg = rhs._cli_last_msg;
	}
	return (*this);
}

void    Client::setSocket(int &sock)
{
    _cli_socket = sock;
}

void    Client::setAddress(sockaddr_in &addr)
{
    _cli_address =  addr;
}

void    Client::setServer(Server &server)
{
    response.setServer(server);
}

const int     &Client::getSocket() const
{
    return (_cli_socket);
}

const Request   &Client::getRequest() const
{
    return (request);
}

const struct sockaddr_in    &Client::getAddress() const
{
    return (_cli_address);
}

const time_t     &Client::getTimeLastMessage() const
{
    return (_cli_last_msg);
}

void        Client::buildResponse()
{
    response.setRequest(this->request);
    response.buildResponse();
}

void             Client::updateTimeLastMessage()
{
    _cli_last_msg = time(NULL);
}

void             Client::clearClient()
{
    response.clearResponse();
    request.clearRequest();
}
