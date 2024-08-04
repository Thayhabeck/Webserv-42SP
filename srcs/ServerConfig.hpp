#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

// Struct que armazena as configurações de uma localização.
struct Location {
    // O caminho da localização.
    std::string path;
    // O diretório raiz da localização.
    std::string root;
    // A lista de arquivos index da localização.
    std::vector<std::string> index;
    // A lista de métodos HTTP permitidos.
    std::vector<std::string> allow_methods;
    // A flag de autoindex.
    bool autoindex;
    // O diretório de armazenamento de upload.
    std::string upload_store;
    // A extensão do CGI.
    std::string cgi_extension;
};

// Struct que armazena as configurações de um servidor.
struct ServerConfig {
    // A lista de nomes de servidor.
    std::vector<std::string> server_names;
    // A porta de escuta.
    int listen_port;
    // A página de erro.
    std::string error_page;
    // O tamanho máximo do corpo do cliente.
    std::string client_max_body_size;
    // O mapeamento de caminhos para localizações.
    std::map<std::string, Location> locations;
};

#endif
