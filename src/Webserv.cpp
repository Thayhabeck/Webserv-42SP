/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: matcardo <matcardo@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 23:20:35 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/13 23:05:53 by matcardo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Webserv.hpp"

Webserv::Webserv(){}
Webserv::~Webserv(){}

void    Webserv::initServers(std::vector<Server> servers)
{
    std::cout << std::endl;
    std::cout << GREEN << "Initializing  Servers..." << RESET << std::endl;
    _servers = servers;
    char buf[INET_ADDRSTRLEN];
    bool    serverDub;
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        serverDub = false;
        for (std::vector<Server>::iterator it2 = _servers.begin(); it2 != it; ++it2)
        {
            if (it2->getHost() == it->getHost() && it2->getPort() == it->getPort())
            {
                it->setFd(it2->getServerFd());
                serverDub = true;
            }
        }
        if (!serverDub)
            it->setupServer();
        std::cout << CYAN "Server Created: ServerName[" << it->getServerName() << 
        "] Host[" << inet_ntop(AF_INET, &it->getHost(), buf, INET_ADDRSTRLEN) << 
        "] Port[" << it->getPort() << "]" RESET << std::endl;
    }
}

void    Webserv::runServers()
{
    fd_set  recv_set_cpy;
    fd_set  write_set_cpy;
    int     select_ret;

    _biggest_fd = 0;
    initializeSets();
    struct timeval timer;
    while (true)
    {
        timer.tv_sec = 1;
        timer.tv_usec = 0;
        recv_set_cpy = _recv_fd_pool;
        write_set_cpy = _write_fd_pool;

        if ( (select_ret = select(_biggest_fd + 1, &recv_set_cpy, &write_set_cpy, NULL, &timer)) < 0 )
        {
            std::cerr << RED "webserv: select error" RESET << std::endl;
            exit(EXIT_FAILURE);
            continue ;
        }
        for (int i = 0; i <= _biggest_fd; ++i)
        {
            if (FD_ISSET(i, &recv_set_cpy) && _servers_map.count(i))
				openCliConnection(_servers_map.find(i)->second);
            else if (FD_ISSET(i, &recv_set_cpy) && _clients_map.count(i))
                readRequest(i, _clients_map[i]);
            else if (FD_ISSET(i, &write_set_cpy) && _clients_map.count(i))
            {
                int cgi_state = _clients_map[i].response.getCgiState();
                if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_in[1], &write_set_cpy))
                    sendCgiBody(_clients_map[i], _clients_map[i].response._cgi_obj);
                else if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_out[0], &recv_set_cpy))
                    readCgiResponse(_clients_map[i], _clients_map[i].response._cgi_obj);
                else if ((cgi_state == 0 || cgi_state == 2)  && FD_ISSET(i, &write_set_cpy))
                    sendResponse(i, _clients_map[i]);
            }
        }
        checkTimeout();
    }
}

void    Webserv::checkTimeout()
{
    for(std::map<int, Client>::iterator it = _clients_map.begin() ; it != _clients_map.end(); ++it)
    {
        if (time(NULL) - it->second.getTimeLastMessage() > CONNECTION_TIMEOUT)
        {
            std::cout << YELLOW "Client " << it->first << " Timeout, Closing Connection.." RESET << std::endl;
            closeCliConnection(it->first);
            return ;
        }
    }
}

void    Webserv::initializeSets()
{
    FD_ZERO(&_recv_fd_pool);
    FD_ZERO(&_write_fd_pool);

    for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        if (listen(it->getServerFd(), N_QUEUED_CONECTIONS) == -1)
        {
            std::cerr << RED "webserv: listen error" RESET << std::endl;
            exit(EXIT_FAILURE);
        }
        if (fcntl(it->getServerFd(), F_SETFL, O_NONBLOCK) < 0)
        {
            std::cerr << RED "webserv: fcntl error" RESET << std::endl;
            exit(EXIT_FAILURE);
        }
        addToSet(it->getServerFd(), _recv_fd_pool);
        _servers_map.insert(std::make_pair(it->getServerFd(), *it));
    }
}

void    Webserv::openCliConnection(Server &serv)
{
	struct sockaddr_in	client_address;
	long				client_address_size = sizeof(client_address);
	int					client_sock;
	Client				new_client(serv);
	char				buf[INET_ADDRSTRLEN];

    if ( (client_sock = accept(serv.getServerFd(), (struct sockaddr *)&client_address,
     (socklen_t*)&client_address_size)) == -1)
    {
        std::cerr << RED "webserv: client connection error" RESET << std::endl;
        return ;
    }
    std::cout << BLUE "New Client Connection From " << inet_ntop(AF_INET, &client_address, buf, INET_ADDRSTRLEN) << 
    " Assigned Socket " << client_sock << RESET << std::endl;

    addToSet(client_sock, _recv_fd_pool);

    if (fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << RED "webserv: fcntl error: " RESET << strerror(errno)<< std::endl;
        removeFromSet(client_sock, _recv_fd_pool);
        close(client_sock);
        return ;
    }
    new_client.setSocket(client_sock);
    if (_clients_map.count(client_sock) != 0)
        _clients_map.erase(client_sock);
    _clients_map.insert(std::make_pair(client_sock, new_client));
}

void    Webserv::closeCliConnection(const int i)
{
    if (FD_ISSET(i, &_write_fd_pool))
        removeFromSet(i, _write_fd_pool);
    if (FD_ISSET(i, &_recv_fd_pool))
        removeFromSet(i, _recv_fd_pool);
    close(i);
    _clients_map.erase(i);
}

void    Webserv::sendResponse(const int &i, Client &c)
{
    int bytes_sent;
    std::string response = c.response.getResponse();
    if (response.length() >= MESSAGE_BUFFER)
        bytes_sent = write(i, response.c_str(), MESSAGE_BUFFER);
    else
        bytes_sent = write(i, response.c_str(), response.length());

    if (bytes_sent < 0)
    {
        std::cerr << RED "webserv: error sending response" RESET << std::endl;
        closeCliConnection(i);
    }
    else if (bytes_sent == 0 || (size_t) bytes_sent == response.length())
    {
        std::cout << CYAN "Response Sent To Socket " << i << ", Stats=" << c.response.getCode() << RESET << std::endl;
        if (c.request.keepConnected() == false || c.request.getErrorCode() || c.response.getCgiState())
        {
            std::cout << YELLOW "Client " << i << " Connection Closed" RESET << std::endl;
            closeCliConnection(i);
        }
        else
        {
            removeFromSet(i, _write_fd_pool);
            addToSet(i, _recv_fd_pool);
            c.clearClient();
        }
    }
    else
    {
        c.updateTimeLastMessage();
        c.response.breakResponse(bytes_sent);
    }
}

void    Webserv::assignServerToClient(Client &client)
{
    for (std::vector<Server>::iterator it = _servers.begin();
        it != _servers.end(); ++it)
    {
        if (client.server.getHost() == it->getHost() &&
            client.server.getPort() == it->getPort() &&
            client.request.getServerName() == it->getServerName())
        {
            client.setServer(*it);
            return ;
        }
    }
}

void    Webserv::readRequest(const int &i, Client &client)
{
	char	buffer[MESSAGE_BUFFER];
	int		bytes_read = 0;

	bytes_read = read(i, buffer, MESSAGE_BUFFER);
	if (bytes_read == 0)
	{
		std::cout << YELLOW "Client " << i << " Closed Connection" RESET << std::endl;
		closeCliConnection(i);
		return ;
	}
	else if (bytes_read < 0)
	{
		std::cerr << RED "webserv: fd " << i << " read error " RESET << std::endl;
		closeCliConnection(i);
		return ;
	}
	else if (bytes_read != 0)
	{
		client.updateTimeLastMessage();
		client.request.feedRequest(buffer, bytes_read);
		memset(buffer, 0, sizeof(buffer));
	}

	if (client.request.isParsed() || client.request.getErrorCode())
	{
		assignServerToClient(client);
		std::cout << CYAN "Request Recived From Socket " << i << ", Method=" << client.request.getMethodStr() << 
		" URI=" << client.request.getPath() << RESET << std::endl;
		client.buildResponse();
		if (client.response.getCgiState())
		{
			handleRequestBody(client);
			addToSet(client.response._cgi_obj.pipe_in[1],  _write_fd_pool);
			addToSet(client.response._cgi_obj.pipe_out[0],  _recv_fd_pool);
		}
		removeFromSet(i, _recv_fd_pool);
		addToSet(i, _write_fd_pool);
	}
}

void    Webserv::handleRequestBody(Client &c)
{
    	if (c.request.getBody().length() == 0)
		{
			std::string tmp;
			std::fstream file;
            c.response._cgi_obj.getCgiPath().c_str();
			std::stringstream ss;
			ss << file.rdbuf();
			tmp = ss.str();
			c.request.setBody(tmp);
		}
}

void    Webserv::sendCgiBody(Client &c, Cgi &cgi)
{
    int bytes_sent;
    std::string &req_body = c.request.getBody();

    if (req_body.length() == 0)
        bytes_sent = 0;
    else if (req_body.length() >= MESSAGE_BUFFER)
        bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), MESSAGE_BUFFER);
    else
        bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), req_body.length());

    if (bytes_sent < 0)
    {
        std::cerr << RED "webserv: error sending to CGI script: " RESET << strerror(errno)<< std::endl;
        removeFromSet(cgi.pipe_in[1], _write_fd_pool);
        close(cgi.pipe_in[1]);
        close(cgi.pipe_out[1]);
        c.response.setResponseError(500);
    }
    else if (bytes_sent == 0 || (size_t) bytes_sent == req_body.length())
    {
        removeFromSet(cgi.pipe_in[1], _write_fd_pool);
        close(cgi.pipe_in[1]);
        close(cgi.pipe_out[1]);
    }
    else
    {
        c.updateTimeLastMessage();
        req_body = req_body.substr(bytes_sent);
    }
}

void    Webserv::readCgiResponse(Client &c, Cgi &cgi)
{
    char    buffer[MESSAGE_BUFFER * 2];
    int     bytes_read = 0;
    bytes_read = read(cgi.pipe_out[0], buffer, MESSAGE_BUFFER* 2);

    if (bytes_read == 0)
    {
        removeFromSet(cgi.pipe_out[0], _recv_fd_pool);
        close(cgi.pipe_in[0]);
        close(cgi.pipe_out[0]);
		int status;
		waitpid(cgi.getCgiPid(), &status, 0);
		if(WEXITSTATUS(status) != 0)
		{
			c.response.setResponseError(502);
		}
        c.response.setCgiState(2);
        if (c.response._response_content.find("HTTP/1.1") == std::string::npos)
		    c.response._response_content.insert(0, "HTTP/1.1 200 OK\r\n");
        return ;
    }
    else if (bytes_read < 0)
    {
        std::cerr << RED "webserv: error reading from CGI script" RESET << std::endl;
        removeFromSet(cgi.pipe_out[0], _recv_fd_pool);
        close(cgi.pipe_in[0]);
        close(cgi.pipe_out[0]);
        c.response.setCgiState(2);
        c.response.setResponseError(500);
        return ;
    }
    else
    {
        c.updateTimeLastMessage();
		c.response._response_content.append(buffer, bytes_read);
		memset(buffer, 0, sizeof(buffer));
    }
}

void	Webserv::addToSet(const int i, fd_set &set)
{
    FD_SET(i, &set);
    if (i > _biggest_fd)
        _biggest_fd = i;
}

void	Webserv::removeFromSet(const int i, fd_set &set)
{
    FD_CLR(i, &set);
    if (i == _biggest_fd)
        _biggest_fd--;
}
