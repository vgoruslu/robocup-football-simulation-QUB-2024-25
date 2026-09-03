#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem> // For directory traversal
#include "nlohmann/json.hpp"

namespace fs = std::filesystem; // Namespace alias for easier usage
using json = nlohmann::json;

struct Shot {
    double x;       // X coordinate of the shot
    double y;       // Y coordinate of the shot
    double xg;      // StatsBomb xG value
};

// Function to parse shot data from a JSON file
std::vector<Shot> parseShotData(const std::string& filename) {
    std::vector<Shot> shots;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return shots;
    }

    json data;
    file >> data;

    for (const auto& event : data) {
        if (event["type"]["name"] == "Shot") {
            Shot shot;
            shot.x = event["location"][0];
            shot.y = event["location"][1];
            shot.xg = event["shot"]["statsbomb_xg"];
            shots.push_back(shot);
        }
    }

    return shots;
}

// Function to get all JSON file names from a specified directory
std::vector<std::string> getFileNamesFromDirectory(const std::string& path) {
    std::vector<std::string> fileNames;

    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.path().extension() == ".json") {
                fileNames.push_back(entry.path().string());
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }

    return fileNames;
}

int main() {
    // Specify the directory containing JSON files
    std::string directoryPath = "C:/Users/james/Documents/uni work/3rd year/PROJECT/xg model/open-data-master/open-data-master/data/events";

    // Get all JSON file names from the directory
    std::vector<std::string> fileNames = getFileNamesFromDirectory(directoryPath);

    if (fileNames.empty()) {
        std::cerr << "No JSON files found in the directory: " << directoryPath << std::endl;
        return 1;
    }

    std::vector<Shot> allShots;

    // Process each file
    for (const auto& fileName : fileNames) {
        std::vector<Shot> shots = parseShotData(fileName);
        allShots.insert(allShots.end(), shots.begin(), shots.end()); // Append shots to allShots
    }

    // Print the collected shot data
    for (const auto& shot : allShots) {
        std::cout << "Shot Location: (" << shot.x << ", " << shot.y << ")"
            << " | xG: " << shot.xg << std::endl;
    }

    return 0;
}
