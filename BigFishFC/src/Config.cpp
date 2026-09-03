#include "Config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

Config parseArguments(int argc, char* argv[]) {
    Config config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            config.host = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            config.port = std::stoi(argv[++i]);
        }
        else if (arg == "--teamname" && i + 1 < argc) {
            config.teamname = argv[++i];
        }
        else if (arg == "--number" && i + 1 < argc) {
            config.number_of_players = std::stoi(argv[++i]);
        }
        else if (arg == "--without-coach") {
            config.use_coach = false;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
        }
    }

    return config;
}

Config loadConfigFromFile(const std::string& filename) {
    Config config;
    std::ifstream file(filename);
    if (file) {
        nlohmann::json json_config;
        file >> json_config;

        config.host = json_config.value("host", "localhost");
        config.port = json_config.value("port", 6000);
        config.teamname = json_config.value("teamname", "Big_Fish_FC");
        config.number_of_players = json_config.value("number_of_players", 11);
        config.use_coach = json_config.value("use_coach", true);
    }
    else {
        std::cerr << "Could not load configuration file: " << filename << std::endl;
    }
    return config;
}
