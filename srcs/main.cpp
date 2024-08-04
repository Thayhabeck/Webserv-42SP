#include "Server.hpp"
#include "Config.hpp"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void startServers(const std::vector<ServerConfig>& serverConfigs) {
    std::vector<pid_t> pids;

    for (size_t i = 0; i < serverConfigs.size(); ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            throw std::runtime_error("Fork failed");
        } else if (pid == 0) { // Child process
            try {
                Server server(serverConfigs[i]);
                server.start();
            } catch (const std::exception& e) {
                std::cerr << "Server error: " << e.what() << std::endl;
                exit(1);
            }
        } else { // Parent process
            pids.push_back(pid);
        }
    }

    // Parent process waits for all child processes
    for (size_t i = 0; i < pids.size(); ++i) {
        int status;
        waitpid(pids[i], &status, 0);
    }
}

int main(int argc, char** argv) {
    std::string config_path = "config/default.conf";
    if (argc == 2) {
        config_path = argv[1];
    }
    
    try {
        Config config(config_path);
        std::vector<ServerConfig> serverConfigs = config.getServerConfigs();
        startServers(serverConfigs);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
