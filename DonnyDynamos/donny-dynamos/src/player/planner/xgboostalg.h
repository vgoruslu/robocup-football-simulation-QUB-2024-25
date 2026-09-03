#ifndef XGBOOST_PREDICT_H
#define XGBOOST_PREDICT_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "nlohmann/json.hpp"
#include <xgboost/c_api.h>
#include <random>

// Define the Point structure for shot location
struct Point {
    double x, y;
};

// Define the Shot structure to hold shot location and its predicted xG
struct Shot {
    Point location;  // x, y position of the shot
    double xg;       // Predicted xG value
};

// Function to parse shot data from a JSON file (e.g., "transformed_shots.json")
std::vector<Shot> parseShotData(const std::string& filename);

// Function to convert shot data into DMatrix (XGBoost format)
DMatrixHandle convertToDMAT(const std::vector<Shot>& shots);

// Function to convert a single shot to DMatrix format for prediction
DMatrixHandle shotToDMatrix(const Shot& shot);

// Function to load and predict xG for a given shot using an XGBoost model
void predictShotxg(Shot& testShot);

// Function to train the XGBoost model with given shot data (for training purposes)
void trainTestNoCV(const std::vector<Shot>& shots, const std::map<std::string, std::string>& params);

// Function to perform k-fold cross-validation for model evaluation
void kFoldCrossValidation(const std::vector<Shot>& shots, int k, const std::map<std::string, std::string>& params);

// Function to generate test shots for evaluation (example positions)
std::vector<Shot> generateTestShots();

#endif // XGBOOST_PREDICT_H
