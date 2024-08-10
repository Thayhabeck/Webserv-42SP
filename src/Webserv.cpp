/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thabeck- <thabeck-@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/09 23:20:35 by thabeck-          #+#    #+#             */
/*   Updated: 2024/08/09 23:52:19 by thabeck-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Webserv.hpp"

Webserv::Webserv(){}
Webserv::~Webserv(){}

/* Inicializa os servidores com as configurações constantes arquivo de configuração */
void    Webserv::initServers(std::vector<Server> servers)
{
    std::cout << std::endl;
    std::cout << GREEN << "Initializing  Servers..." << RESET << std::endl;
    _servers = servers;
    char buf[INET_ADDRSTRLEN];
    // serverDub é uma flag que verifica se o servidor já foi criado
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
        // inet_ntop converte o endereço binário para string
        std::cout << CYAN "Server Created: ServerName[" << it->getServerName() << 
        "] Host[" << inet_ntop(AF_INET, &it->getHost(), buf, INET_ADDRSTRLEN) << 
        "] Port[" << it->getPort() << "]" RESET << std::endl;
    }
}

/* Executa o loop principal que passa por todos os descritores de arquivo de 0 até o maior descritor de arquivo no conjunto.
 * - verifica os descritores de arquivo retornados de select():
 *      se fd do servidor --> aceita novo cliente
 *      se fd do cliente no conjunto de leitura --> lê mensagem do cliente
 *      se fd do cliente no conjunto de gravação:
 *          1- Se for uma resposta CGI e o corpo ainda não foi enviado ao processo filho CGI --> Envia corpo da requisição ao processo filho CGI.
 *          2- Se for uma resposta CGI e o corpo foi enviado ao processo filho CGI --> Lê a saída do processo filho CGI.
 *          3- Se for uma resposta normal --> Envia resposta ao cliente.
 * - os sockets de servidores e clientes serão adicionados ao _recv_set_pool inicialmente,
 *   depois disso, quando uma solicitação for totalmente analisada, o soquete será movido para _write_set_pool
 */
void    Webserv::runServers()
{
    fd_set  recv_set_cpy;
    fd_set  write_set_cpy;
    int     select_ret;

    _biggest_fd = 0;
    // Inicializa os conjuntos de descritores de arquivo para servidores e clientes
    initializeSets();
    struct timeval timer;
    while (true)
    {
        timer.tv_sec = 1;
        timer.tv_usec = 0;
        recv_set_cpy = _recv_fd_pool;
        write_set_cpy = _write_fd_pool;

        // select() é uma chamada de sistema que monitora os descritores de arquivo e 
        // aguarda até que um deles esteja pronto para ser lido ou gravado.
        // O primeiro argumento é o maior descritor de arquivo + 1
        // O segundo argumento é o conjunto de descritores de arquivo que estão prontos para leitura
        // O terceiro argumento é o conjunto de descritores de arquivo que estão prontos para gravação
        // O quarto argumento é o conjunto de descritores de arquivo que estão prontos para exceção
        // O quinto argumento é um temporizador
        // select() retorna o número de descritores de arquivo prontos para leitura ou gravação
        if ( (select_ret = select(_biggest_fd + 1, &recv_set_cpy, &write_set_cpy, NULL, &timer)) < 0 )
        {
            std::cerr << RED "webserv: select error: " RESET << strerror(errno) << std::endl;
            exit(1);
            continue ;
        }
        for (int i = 0; i <= _biggest_fd; ++i)
        {
            // FD_ISSET é uma macro que verifica se um descritor de arquivo está no conjunto de descritores de arquivo recv_set_cpy
            if (FD_ISSET(i, &recv_set_cpy) && _servers_map.count(i)) //Aceita nova conexão de cliente
                openCliConnection(_servers_map.find(i)->second);
            else if (FD_ISSET(i, &recv_set_cpy) && _clients_map.count(i)) //Lê mensagem do cliente
                readRequest(i, _clients_map[i]);
            else if (FD_ISSET(i, &write_set_cpy) && _clients_map.count(i)) //Envia resposta ao cliente
            {
                int cgi_state = _clients_map[i].response.getCgiState(); // 0->NoCGI 1->CGI write/read to/from script 2-CGI read/write done
                if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_in[1], &write_set_cpy)) //Envia corpo da requisição ao script CGI
                    sendCgiBody(_clients_map[i], _clients_map[i].response._cgi_obj);
                else if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_out[0], &recv_set_cpy)) //Lê resposta do script CGI
                    readCgiResponse(_clients_map[i], _clients_map[i].response._cgi_obj);
                else if ((cgi_state == 0 || cgi_state == 2)  && FD_ISSET(i, &write_set_cpy)) //Envia resposta ao cliente
                    sendResponse(i, _clients_map[i]);
            }
        }
        checkTimeout();
    }
}

/* Verifica o tempo passado para os clientes desde a última mensagem, se mais do que CONNECTION_TIMEOUT, fecha a conexão */
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

/* inicializa os conjuntos de descritores de arquivo para servidores e clientes */
void    Webserv::initializeSets()
{
    //FD_ZERO é uma macro que limpa o conjunto de descritores de arquivo 
    FD_ZERO(&_recv_fd_pool);
    FD_ZERO(&_write_fd_pool);

    // _recv_fd_pool set é um conjunto de descritores de arquivo que estão prontos para leitura
    // Aqui, adicionamos todos os servidores ao conjunto de descritores de arquivo de leitura
    for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        // listen() é uma chamada de sistema que coloca o socket em um estado passivo, onde ele espera por conexões de entrada.
        // O segundo argumento é o número de conexões que podem ser enfileiradas antes de serem aceitas.
        if (listen(it->getServerFd(), 512) == -1)
        {
            std::cerr << RED "webserv: listen error: " RESET << strerror(errno)<< std::endl;
            exit(EXIT_FAILURE);
        }
        // fcntl() é uma chamada de sistema que manipula o descritor de arquivo
        // F_SETFL é uma operação que define o status do descritor de arquivo
        // O_NONBLOCK é uma flag que define o descritor de arquivo como não bloqueante
        // Este statement é para garantir que o socket do servidor seja não bloqueante, que quer diz que ele não espera por uma resposta
        if (fcntl(it->getServerFd(), F_SETFL, O_NONBLOCK) < 0)
        {
            //Logger::logMsg(RED, CONSOLE_OUTPUT, "webserv: fcntl error: %s   Closing....", strerror(errno));
            std::cerr << RED "webserv: fcntl error: " RESET << strerror(errno)<< std::endl;
            exit(EXIT_FAILURE);
        }
        // addToSet adiciona o descritor de arquivo ao conjunto de descritores de arquivo
        addToSet(it->getServerFd(), _recv_fd_pool);
        _servers_map.insert(std::make_pair(it->getServerFd(), *it));
    }
    // _biggest_fd é o maior descritor de arquivo, que é o último descritor de arquivo adicionado ao conjunto de descritores de arquivo
    _biggest_fd = _servers.back().getServerFd();
}

/* Aceita nova conexão de entrada, cria um novo objeto Cliente e o adiciona o cliente ao _clients_map e seu socket ao _recv_fd_pool */
void    Webserv::openCliConnection(Server &serv)
{
    // sockaddr_in é uma estrutura que contém um endereço de internet
    // client_address é uma estrutura que contém o endereço do cliente
    struct sockaddr_in client_address;
    long  client_address_size = sizeof(client_address);
    int client_sock;
    Client  new_client(serv);
    // INET_ADDRSTRLEN é o tamanho máximo de uma string de endereço de internet
    char    buf[INET_ADDRSTRLEN];

    // accept() é uma chamada de sistema que aceita uma conexão de entrada em um socket
    // O primeiro argumento é o descritor de arquivo do socket do servidor
    // O segundo argumento é um ponteiro para a estrutura sockaddr do cliente
    // O terceiro argumento é um ponteiro para o tamanho da estrutura sockaddr
    // accept() retorna o descritor de arquivo do socket do cliente
    if ( (client_sock = accept(serv.getServerFd(), (struct sockaddr *)&client_address,
     (socklen_t*)&client_address_size)) == -1)
    {
        std::cerr << RED "webserv: client connection error: " RESET << strerror(errno)<< std::endl;
        return ;
    }
    std::cout << CYAN "New Client Connection From " << inet_ntop(AF_INET, &client_address, buf, INET_ADDRSTRLEN) << 
    " Assigned Socket " << client_sock << RESET << std::endl;

    // addToSet adiciona o descritor de arquivo ao conjunto de descritores de arquivo para leiura
    addToSet(client_sock, _recv_fd_pool);

    // fcntl() é uma chamada de sistema que manipula o descritor de arquivo
    // F_SETFL é uma operação que define o status do descritor de arquivo como não bloqueante
    // O_NONBLOCK é uma flag que define o descritor de arquivo como não bloqueante
    if (fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cerr << RED "webserv: fcntl error: " RESET << strerror(errno)<< std::endl;
        removeFromSet(client_sock, _recv_fd_pool);
        close(client_sock);
        return ;
    }
    // setSocket é uma função que define o descritor de arquivo do cliente
    new_client.setSocket(client_sock);
    if (_clients_map.count(client_sock) != 0)
        _clients_map.erase(client_sock);
    _clients_map.insert(std::make_pair(client_sock, new_client));
}

/* Fecha a conexão do fd e remove o objeto cliente associado do _clients_map */
void    Webserv::closeCliConnection(const int i)
{
    if (FD_ISSET(i, &_write_fd_pool))
        removeFromSet(i, _write_fd_pool);
    if (FD_ISSET(i, &_recv_fd_pool))
        removeFromSet(i, _recv_fd_pool);
    close(i);
    _clients_map.erase(i);
}

/**
 * Monta a resposta e a envia para o cliente.
 * Se não encontrar nenhum erro na solicitação e o valor do cabeçalho de conexão for keep-alive,
 * a conexão é mantida, caso contrário, a conexão será fechada.
 */
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
        std::cerr << RED "webserv: error sending response: " RESET << strerror(errno)<< std::endl;
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

/* Atribui a configuração do servidor a um cliente com base no cabeçalho Host na solicitação e no nome do servidor */
void    Webserv::assignServerToClient(Client &c)
{
    for (std::vector<Server>::iterator it = _servers.begin();
        it != _servers.end(); ++it)
    {
        if (c.server.getHost() == it->getHost() &&
            c.server.getPort() == it->getPort() &&
            c.request.getServerName() == it->getServerName())
        {
            c.setServer(*it);
            return ;
        }
    }
}

/**
 * - Lê a requisição do cliente e a armazena em um buffer.
 * - Se a requisição estiver completa, a resposta será construída e o socket será movido de _recv_fd_pool para _write_fd_pool
 * e a resposta enviada para a próxima iteração do select().
 */
void    Webserv::readRequest(const int &i, Client &c)
{
    char    buffer[MESSAGE_BUFFER];
    int     bytes_read = 0;
    bytes_read = read(i, buffer, MESSAGE_BUFFER);
    if (bytes_read == 0)
    {
        std::cout << YELLOW "Client " << i << " Closed Connection" RESET << std::endl;
        closeCliConnection(i);
        return ;
    }
    else if (bytes_read < 0)
    {
        std::cerr << RED "webserv: fd " << i << " read error " RESET  << strerror(errno) << std::endl;
        closeCliConnection(i);
        return ;
    }
    else if (bytes_read != 0)
    {
        c.updateTimeLastMessage();
        c.request.feedRequest(buffer, bytes_read);
        // Memset é uma função que preenche um bloco de memória com um valor específico
        memset(buffer, 0, sizeof(buffer));
    }

    if (c.request.isParsed() || c.request.getErrorCode()) // 1 = parsing completed and we can work on the response.
    {
        assignServerToClient(c);
        std::cout << CYAN "Request Recived From Socket " << i << ", Method=" << c.request.getMethodStr() << 
        " URI=" << c.request.getPath() << RESET << std::endl;
        c.buildResponse();
        if (c.response.getCgiState())
        {
            handleRequestBody(c);
            addToSet(c.response._cgi_obj.pipe_in[1],  _write_fd_pool);
            addToSet(c.response._cgi_obj.pipe_out[0],  _recv_fd_pool);
        }
        removeFromSet(i, _recv_fd_pool);
        addToSet(i, _write_fd_pool);
    }
}

// Se o corpo da requisição estiver vazio, ele será preenchido com o conteúdo do arquivo
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

/* Send request body to CGI script */
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

/* Lê a saída produzida pelo script CGI */
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
        std::cerr << RED "webserv: error reading from CGI script: " RESET << strerror(errno)<< std::endl;
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
    //FD_SET é uma macro que adiciona um descritor de arquivo ao conjunto de descritores de arquivo
    FD_SET(i, &set);
    if (i > _biggest_fd)
        _biggest_fd = i;
}

void	Webserv::removeFromSet(const int i, fd_set &set)
{
    //FD_CLR é uma macro que remove um descritor de arquivo do conjunto de descritores de arquivo
    FD_CLR(i, &set);
    if (i == _biggest_fd)
        _biggest_fd--;
}
