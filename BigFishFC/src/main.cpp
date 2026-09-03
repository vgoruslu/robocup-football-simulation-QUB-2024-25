#include <iostream>
#include "Simulator.h"
#include "Config.h"

int main(int argc, char* argv[]) {
    Config config = parseArguments(argc, argv);
    Config fileConfig = loadConfigFromFile("config.json");

    // Override file configuration with command-line arguments
    if (!config.host.empty()) fileConfig.host = config.host;
    if (config.port != 0) fileConfig.port = config.port;

    Simulator simulator(fileConfig.teamname, fileConfig.number_of_players, fileConfig.use_coach);
    simulator.run();

    return 0;
}
