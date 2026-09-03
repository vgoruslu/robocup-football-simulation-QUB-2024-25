#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    std::string host = "localhost";
    int port = 6000;
    std::string teamname = "Big_Fish_FC";
    int number_of_players = 11;
    bool use_coach = true;
};

// Parses command-line arguments
Config parseArguments(int argc, char* argv[]);

// Loads configuration from a JSON file
Config loadConfigFromFile(const std::string& filename);

#endif // CONFIG_H
