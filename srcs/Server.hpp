#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"

class Server {
public:
    Server(const ServerConfig& config);
    ~Server();

    void start();

private:
    void initServer();
    void handleClient(int client_fd);
    
    int server_fd;
    ServerConfig config;
    std::map<int, std::string> client_requests;
    std::vector<struct pollfd> poll_fds;
};

#endif // SERVER_HPP
