#include "Server.hpp"
#include "Utils.hpp"
#include <fstream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>

Server::Server(const ServerConfig& config) : config(config) {
    initServer();
}

Server::~Server() {
    close(server_fd);
}

// initServer() é uma função privada que inicializa o servidor.
// A função cria um socket, associa um endereço a ele e coloca o socket em um estado passivo.
// Se a chamada de sistema falhar, a função lança uma exceção.
void Server::initServer() {
    // A função socket() cria um socket e retorna um descritor de arquivo que pode ser usado em chamadas de sistema posteriores.
    // O primeiro argumento é o domínio do socket, que é AF_INET para IPv4.
    // O segundo argumento é o tipo de socket, que é SOCK_STREAM para TCP.
    // O terceiro argumento é o protocolo, que é 0 para o protocolo padrão.
    // Se a chamada de sistema falhar, a função socket() retorna -1.
    // Um socket é um ponto final de comunicação bidirecional entre dois programas em uma rede.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Socket creation failed");
    }

    int opt = 1;
    // A função setsockopt() define opções para um socket.
    // O primeiro argumento é o descritor de arquivo do socket.
    // O segundo argumento é o nível de protocolo, que é SOL_SOCKET para o nível de socket.
    // O terceiro argumento é a opção a ser definida, que é SO_REUSEADDR para reutilizar o endereço.
    // O quarto argumento é um ponteiro para o valor da opção.
    // O quinto argumento é o tamanho do valor da opção.
    // Se a chamada de sistema falhar, a função setsockopt() retorna -1.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("Setsockopt failed");
    }

    // a struct sockaddr_in é usada para especificar um endereço de socket para a família de protocolos AF_INET.
    // O membro sin_family deve ser AF_INET. AF_INET é a família de protocolos para IPv4.
    // O membro sin_addr.s_addr é o endereço IP do host. INADDR_ANY é um endereço IP especial 
    // que significa que o socket aceita conexões em todas as interfaces de rede.
    // O membro sin_port é o número da porta. A função htons() converte o número da porta para o formato de rede.
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.listen_port);

    // A função bind() associa um endereço a um socket.
    // O primeiro argumento é o descritor de arquivo do socket.
    // O segundo argumento é um ponteiro para a estrutura sockaddr que contém o endereço a ser associado.
    // O terceiro argumento é o tamanho da estrutura sockaddr.
    // Se a chamada de sistema falhar, a função bind() retorna -1.
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        throw std::runtime_error("Bind failed");
    }

    // A função listen() coloca o socket em um estado passivo, onde ele espera por conexões de entrada.
    // O primeiro argumento é o descritor de arquivo do socket.
    // O segundo argumento é o número máximo de conexões pendentes que podem ser enfileiradas.
    // Se a chamada de sistema falhar, a função listen() retorna -1.
    if (listen(server_fd, 10) == -1) {
        throw std::runtime_error("Listen failed");
    }

    struct pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);
}

// start() é uma função pública que inicia o servidor.
// A função entra em um loop infinito que aguarda eventos em descritores de arquivo.
// Se um evento de leitura ocorrer em um descritor de arquivo, a função handleClient() é chamada para lidar com o cliente.
// Se a chamada de sistema falhar, a função lança uma exceção.
void Server::start() {
    while (true) {
        // A função poll() aguarda até que um evento ocorra em um ou mais descritores de arquivo.
        // O primeiro argumento é um ponteiro para um array de estruturas pollfd.
        // O segundo argumento é o número de elementos no array.
        // O terceiro argumento é o tempo limite em milissegundos. -1 significa esperar indefinidamente.
        // Se a chamada de sistema falhar, a função poll() retorna -1.
        int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_count == -1) {
            throw std::runtime_error("Poll failed");
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == server_fd) {
                    // A função accept() aceita uma conexão de entrada em um socket.
                    // O primeiro argumento é o descritor de arquivo do socket.
                    // O segundo e terceiro argumentos são ponteiros para estruturas sockaddr que contêm o endereço do cliente.
                    // Se a chamada de sistema falhar, a função accept() retorna -1.
                    int client_fd = accept(server_fd, NULL, NULL);
                    if (client_fd == -1) {
                        throw std::runtime_error("Accept failed");
                    }
                    struct pollfd client_pollfd;
                    client_pollfd.fd = client_fd;
                    client_pollfd.events = POLLIN;
                    poll_fds.push_back(client_pollfd);
                } else {
                    handleClient(poll_fds[i].fd);
                }
            }
        }
    }
}

// handleClient() é uma função privada que lida com um cliente.
// A função lê a solicitação do cliente, processa-a e envia uma resposta de volta ao cliente.
// Se a chamada de sistema falhar, a função fecha o descritor de arquivo do cliente e remove 
// o descritor de arquivo da lista de descritores de arquivo.
void Server::handleClient(int client_fd) {
    char buffer[1024] = {0};
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        for (std::vector<struct pollfd>::iterator it = poll_fds.begin(); it != poll_fds.end(); ++it) {
            if (it->fd == client_fd) {
                poll_fds.erase(it);
                break;
            }
        }
    } else {
        buffer[bytes_read] = '\0';
        std::string request(buffer);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
        write(client_fd, response.c_str(), response.size());
    }
}
