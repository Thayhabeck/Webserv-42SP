#include "Config.hpp"
#include "Utils.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

Config::Config(const std::string& config_path) {
    loadConfig(config_path);
}

// loadConfig() é uma função privada que lê o conteúdo do arquivo de configuração e o divide em blocos de servidor.
// Cada bloco de servidor é então analisado para extrair as configurações do servidor.
// As configurações do servidor são armazenadas em um vetor de ServerConfig.
// A função loadConfig() lança uma exceção se não conseguir abrir o arquivo de configuração.
void Config::loadConfig(const std::string& config_path) {
    std::ifstream config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::runtime_error("Cannot open config file");
    }

    std::stringstream buffer;
    buffer << config_file.rdbuf();
    std::string config_content = buffer.str();
    
    std::vector<std::string> server_configs = splitServers(config_content);
    for (size_t i = 0; i < server_configs.size(); ++i) {
        ServerConfig server_config;
        parseServerBlock(server_configs[i], server_config);
        servers.push_back(server_config);
    }
}

// splitServers() é uma função privada que divide o conteúdo do arquivo de configuração em blocos de servidor.
// Um bloco de servidor começa com a linha "server {" e termina com a linha "}".
// A função splitServers() retorna um vetor de strings, onde cada string é um bloco de servidor.
std::vector<std::string> Config::splitServers(const std::string& config_content) {
    std::vector<std::string> server_configs;
    std::string server_start = "server {";
    std::string server_end = "}";

    size_t start_pos = 0;
    while ((start_pos = config_content.find(server_start, start_pos)) != std::string::npos) {
        size_t end_pos = config_content.find(server_end, start_pos) + server_end.length();
        server_configs.push_back(config_content.substr(start_pos, end_pos - start_pos));
        start_pos = end_pos;
    }

    return server_configs;
}

// parseServerBlock() é uma função privada que analisa um bloco de servidor para extrair as configurações do servidor.
// As configurações do servidor são armazenadas em uma instância de ServerConfig.
// A função parseServerBlock() lança uma exceção se encontrar um erro de configuração inválida.
void Config::parseServerBlock(const std::string& server_block, ServerConfig& server_config) {
    std::istringstream config_stream(server_block);
    std::string line;
    Location current_location;
    bool inside_location = false;

    while (std::getline(config_stream, line)) {
        line = trim(line); // Adicione uma função trim para remover espaços em branco no início e no final
        if (line.empty() || line[0] == '#') {
            continue; // Ignore linhas vazias e comentários
        }
        if (line.find("server_name") != std::string::npos) {
            server_config.server_names = split(line.substr(line.find(" ") + 1), ' ');
        } else if (line.find("listen") != std::string::npos) {
            std::string port_str = trim(line.substr(line.find(" ") + 1));
            std::stringstream ss(port_str);
            ss >> server_config.listen_port;
            if (ss.fail() || server_config.listen_port <= 0 || server_config.listen_port > 65535) {
                throw std::runtime_error("Invalid port number: " + port_str);
            }
        } else if (line.find("error_page") != std::string::npos) {
            server_config.error_page = trim(line.substr(line.find(" ") + 1));
        } else if (line.find("client_max_body_size") != std::string::npos) {
            server_config.client_max_body_size = trim(line.substr(line.find(" ") + 1));
        } else if (line.find("location") != std::string::npos) {
            if (inside_location) {
                server_config.locations[current_location.path] = current_location;
                current_location = Location();
            }
            inside_location = true;
            size_t path_end = line.find(" ", line.find(" ") + 1);
            current_location.path = trim(line.substr(line.find(" ") + 1, path_end - line.find(" ") - 1));
            if (line.find(".php") != std::string::npos) {
                current_location.cgi_extension = ".php";
            }
        } else if (line.find("root") != std::string::npos) {
            current_location.root = trim(line.substr(line.find(" ") + 1));
        } else if (line.find("index") != std::string::npos) {
            current_location.index = split(trim(line.substr(line.find(" ") + 1)), ' ');
        } else if (line.find("allow") != std::string::npos) {
            current_location.allow_methods = split(trim(line.substr(line.find(" ") + 1)), ' ');
        } else if (line.find("autoindex") != std::string::npos) {
            std::string autoindex_value = trim(line.substr(line.find(" ") + 1));
            current_location.autoindex = (autoindex_value == "on");
        } else if (line.find("upload_store") != std::string::npos) {
            current_location.upload_store = trim(line.substr(line.find(" ") + 1));
        }
    }

    if (inside_location) {
        server_config.locations[current_location.path] = current_location;
    }
}

std::vector<ServerConfig> Config::getServerConfigs() const {
    return servers;
}
