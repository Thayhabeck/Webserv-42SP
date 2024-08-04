#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <vector>
#include <string>
#include "ServerConfig.hpp"

class Config {
public:
    Config(const std::string& config_path);
    std::vector<ServerConfig> getServerConfigs() const;

private:
    void loadConfig(const std::string& config_path);
    std::vector<std::string> splitServers(const std::string& config_content);
    void parseServerBlock(const std::string& server_block, ServerConfig& server_config);

    std::vector<ServerConfig> servers;
};

#endif
