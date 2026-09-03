#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem> // For directory traversal
#include "nlohmann/json.hpp"

namespace fs = std::filesystem; // Namespace alias for easier usage
using json = nlohmann::json;

struct Point {
    double x, y;
};

struct Shot {
    Point location; // shot location
    double xg;      // StatsBomb xG value
};

Point transformCoordinates(const Point& input) {
    // Source pitch dimensions
    const double sourceWidth = 120.0;
    const double sourceHeight = 80.0;

    // Target pitch dimensions
    const double targetWidth = 104.0;
    const double targetHeight = 66.0;

    Point output;

    // Scale and shift origin
    output.x = (input.x / sourceWidth) * targetWidth - (targetWidth / 2.0);
    output.y = (targetHeight / 2.0) - (input.y / sourceHeight) * targetHeight;

    return output;
}

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
            Point rawLocation = { event["location"][0], event["location"][1] };
            shot.location = transformCoordinates(rawLocation);
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

void writeShotsToJSON(const std::vector<Shot>& allShots, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    json jShots = json::array();

    // Add each shot to the JSON array
    for (const auto& shot : allShots) {
        json jShot;
        jShot["location"]["x"] = shot.location.x;
        jShot["location"]["y"] = shot.location.y;
        jShot["xg"] = shot.xg;
        jShots.push_back(jShot);
    }

    // Write the JSON data to the file
    file << jShots.dump(4); // Pretty print with 4-space indentation
    file.close();
    std::cout << "Shots data written to " << filename << std::endl;
}


int main() {
    // Specify the directory containing JSON files
    std::string directoryPath = "./open-data-master/open-data-master/data/events";

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

    // output 
    /*for (const auto& shot : allShots) {
        std::cout << "Transformed Shot Location: (" << shot.location.x << ", " << shot.location.y << ")"
            << " | xG: " << shot.xg << std::endl;
    }
    */

    writeShotsToJSON(allShots, "transformed_shots.json");

    return 0;
}