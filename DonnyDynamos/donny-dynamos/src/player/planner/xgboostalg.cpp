#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include "nlohmann/json.hpp"
#include <xgboost/c_api.h>
#include <random>
#include <set>

// g++ -o xgboostalg xgboostalg.cpp -I/home/james/xgboost/include -L/home/james/xgboost/lib -lxgboost
// export LD_LIBRARY_PATH=/home/james/xgboost/lib:$LD_LIBRARY_PATH
// ./xgboostalg

// VIRTUAL MACHINE
// g++ -o xgboostalg xgboostalg.cpp \
//    -I/home/ubuntu/Downloads/xgboost-src/include \
//    -I/usr/include/nlohmann \
//    -L/home/ubuntu/Downloads/xgboost-src/lib \
//    -lxgboost

// export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ubuntu/Downloads/xgboost-src/lib
// ./xgboostalg


using json = nlohmann::json;

struct Point {
	double x, y;
};

struct Shot {
	Point location;
	double xg;
};

double calculateMSE(const std::vector<double>& predictedXG, const std::vector<double>& trueXG) {
	if (predictedXG.size() != trueXG.size()) {
		std::cerr << "Error: Mismatch in predicted and ground truth sizes." << std::endl;
		return -1.0;
	}

	double MSE = 0.0;
	size_t numShots = predictedXG.size();

	for (size_t i = 0; i < numShots; ++i) {
		double error = predictedXG[i] - trueXG[i];
		MSE += error * error;
	}

	return MSE / numShots; // Mean Squared Error
}


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
		Shot shot;
		shot.location.x = event["location"]["x"].get<double>();
		shot.location.y = event["location"]["y"].get<double>();
		shot.xg = event["xg"].get<double>();
		shots.push_back(shot);
	}

	return shots;
}

DMatrixHandle convertToDMAT(const std::vector<Shot>& shots) {
	size_t num_samples = shots.size();
	size_t num_features = 3; // x, y and distance

	std::vector<float> features;
	std::vector<float> labels;

	for (const auto& shot : shots) {
		double distance = std::sqrt(std::pow(shot.location.x, 2) + std::pow(shot.location.y, 2));
		features.push_back(static_cast<float>(shot.location.x));
		features.push_back(static_cast<float>(shot.location.y));
		features.push_back(static_cast<float>(distance));
		labels.push_back(static_cast<float>(shot.xg));
	}

	DMatrixHandle dtrain;
	int result = XGDMatrixCreateFromMat(features.data(), num_samples, num_features, -1, &dtrain);
	if (result != 0) {
		std::cerr << "Failed to create DMatrix." << std::endl;
		return nullptr;
	}

	result = XGDMatrixSetFloatInfo(dtrain, "label", labels.data(), labels.size());
	if (result != 0) {
		std::cerr << "Failed to set labels for DMatrix." << std::endl;
		XGDMatrixFree(dtrain);
		return nullptr;
	}

	return dtrain;
}

DMatrixHandle convertToDMATweights(const std::vector<Shot>& shots) {
	size_t num_samples = shots.size();
	size_t num_features = 3; // x, y and distance

	std::vector<float> features;
	std::vector<float> labels;
	std::vector<float> weights;

	for (const auto& shot : shots) {
		double distance = std::sqrt(std::pow(shot.location.x, 2) + std::pow(shot.location.y, 2));
		features.push_back(static_cast<float>(shot.location.x));
		features.push_back(static_cast<float>(shot.location.y));
		features.push_back(static_cast<float>(distance));

		// Push the label (xG) into the labels vector
		labels.push_back(static_cast<float>(shot.xg));

		// Initialize the weight as the xG value
		float weight = static_cast<float>(shot.xg);

		// Boost weight for wider y values - change this to boost by x amount for y > y value etc.
		if (abs(shot.location.y) < 20) {
			weight *= 1.0 + 10000000 * std::exp(-0.009 * std::abs(shot.location.y));
			if (abs(shot.location.x) > 40) {
				weight *= 1.0 + 20 * std::exp(-0.05 * std::abs(shot.location.x));
			}
		}
		else if (abs(shot.location.y) < 35) {
			weight *= 1.0 + 500000 * std::exp(-0.01 * std::abs(shot.location.y));
		}
		else {
			weight *= 1.0 + 10000 * std::exp(-0.01 * std::abs(shot.location.y));
		}
		// Boost weight for higher x values - change this to boost by x amount for x > y value etc.
		if (abs(shot.location.x) > 40) {
			weight *= 1.0 + 100000000 * std::exp(-0.007 * std::abs(shot.location.x));
		}
		else if (abs(shot.location.x) > 30) {
			weight *= 1.0 + 100000000 * std::exp(-0.009 * std::abs(shot.location.x));
		}
		else {
			weight *= 1.0 + 1000000 * std::exp(-0.01 * std::abs(shot.location.x));
		}

		// Boost weight for higher xG values - change this to boost by x amount for xG > y value etc.
		if (abs(shot.xg) > 0.8) {
			weight *= 1.0 + 100000000000 * std::exp(-0.001 * std::abs(shot.xg));
		}
		else if (abs(shot.xg) > 0.6) {
			weight *= 1.0 + 50000000 * std::exp(-0.001 * std::abs(shot.xg));
		}
		else {
			weight *= 1.0 + 1000 * std::exp(-0.01 * std::abs(shot.xg));
		}



		

		// Push the final weight into the weights vector
		weights.push_back(weight);
	}

	DMatrixHandle dtrain;
	int result = XGDMatrixCreateFromMat(features.data(), num_samples, num_features, -1, &dtrain);
	if (result != 0) {
		std::cerr << "Failed to create DMatrix." << std::endl;
		return nullptr;
	}

	result = XGDMatrixSetFloatInfo(dtrain, "label", labels.data(), labels.size());
	if (result != 0) {
		std::cerr << "Failed to set labels for DMatrix." << std::endl;
		XGDMatrixFree(dtrain);
		return nullptr;
	}

	result = XGDMatrixSetFloatInfo(dtrain, "weight", weights.data(), weights.size()); // Set weights
	if (result != 0) {
		std::cerr << "Failed to set weights for DMatrix." << std::endl;
		XGDMatrixFree(dtrain);
		return nullptr;
	}

	return dtrain;
}

std::vector<Shot> generateTestShots() {
	std::vector<Shot> testShots;
	std::vector<double> distances = { 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0 };
	double yMin = -34.0;
	double yMax = 34.0;
	int numYPositions = 10;

	// To avoid duplicates, use a set to track positions.
	std::set<std::pair<double, double>> uniqueLocations;

	for (double distance : distances) {
		for (int i = 0; i <= numYPositions; ++i) {
			double yPosition = yMin + (i * (yMax - yMin) / numYPositions);

			// Validate the shot location before adding it
			if (std::isfinite(distance) && std::isfinite(yPosition)) {
				if (uniqueLocations.insert({ distance, yPosition }).second) {  // Only insert if not already present.
					testShots.push_back({ {distance, yPosition}, 0.0 });
				}
			}
		}
	}

	return testShots;
}


void trainTestNoCV(const std::vector<Shot>& shots, const std::map<std::string, std::string>& params) {
	DMatrixHandle dtrain = convertToDMATweights(shots);
	if (!dtrain) {
		std::cerr << "Error converting shots to DMatrix." << std::endl;
		return;
	}
	
	// Create a Booster and set the DMatrix
	BoosterHandle booster;
	XGBoosterCreate(&dtrain, 1, &booster);
	for (const auto& param : params) {
		XGBoosterSetParam(booster, param.first.c_str(), param.second.c_str());
	}

	// Train the model
	int num_rounds = 200;
	for (int i = 0; i < num_rounds; ++i) {
		XGBoosterUpdateOneIter(booster, i, dtrain);
	}
	const char* modelFile = "/home/ubuntu/Downloads/james-young-football-simulator-sw-for-robocup-master/xg model/xgboostModel.json";
	XGBoosterSaveModel(booster, modelFile);

	std::cout << "Model training completed and written to file." << std::endl;


	// get a test set
	std::vector<Shot> testShots = generateTestShots();
	DMatrixHandle dtest = convertToDMAT(testShots);
	if (!dtest) {
		std::cerr << "Error converting test shots to DMatrix." << std::endl;
		XGBoosterFree(booster);
		XGDMatrixFree(dtrain);
		return;
	}

	// run some predictions on the test set
	bst_ulong out_len;
	const float* predictions;
	int result = XGBoosterPredict(booster, dtest, 0, 0, 0, &out_len, &predictions);
	if (result != 0) {
		std::cerr << "Prediction failed." << std::endl;
	}
	else {
		std::cout << "Predictions for test shots:" << std::endl;
		for (size_t i = 0; i < out_len; ++i) {
			std::cout << "Test Shot " << i + 1 << " Location: (" << testShots[i].location.x
				<< ", " << testShots[i].location.y << ") | Predicted xG: "
				<< predictions[i] << std::endl;
		}
	}

		std::vector<double> predictedXG;
		std::vector<double> trueXG;

		for (size_t i = 0; i < out_len; ++i) {
			predictedXG.push_back(predictions[i]);
			trueXG.push_back(testShots[i].xg);  // Extracting actual xG
		}

		// Compute MSE
		double MSE = calculateMSE(predictedXG, trueXG);
		std::cout << "Mean Squared Error: " << MSE << std::endl;


	// Cleanup
	XGBoosterFree(booster);
	XGDMatrixFree(dtrain);
}

void kFoldCrossValidation(const std::vector<Shot>& shots, int k, const std::map<std::string, std::string>& params) {
	int fold_size = shots.size() / k;
	std::vector<int> indices(shots.size());
	std::iota(indices.begin(), indices.end(), 0); // Create indices from 0 to shots.size() - 1

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(indices.begin(), indices.end(), g); // Shuffle the indices

	double total_accuracy = 0.0;
	double total_recall = 0.0;
	double total_precision = 0.0;

	// Perform k-fold cross-validation
	for (int fold = 0; fold < k; ++fold) {
		std::vector<Shot> trainShots;
		std::vector<Shot> testShots;

		// Split data into training and testing sets
		for (int i = 0; i < shots.size(); ++i) {
			if (i >= fold * fold_size && i < (fold + 1) * fold_size) {
				testShots.push_back(shots[indices[i]]);
			}
			else {
				trainShots.push_back(shots[indices[i]]);
			}
		}

		// Convert train and test sets to DMatrix
		DMatrixHandle dtrain = convertToDMATweights(trainShots);
		DMatrixHandle dtest = convertToDMAT(testShots);

		if (!dtrain || !dtest) {
			std::cerr << "Error converting data to DMatrix." << std::endl;
			continue;
		}

		// Create and train the model
		BoosterHandle booster;
		XGBoosterCreate(&dtrain, 1, &booster);
		for (const auto& param : params) {
			XGBoosterSetParam(booster, param.first.c_str(), param.second.c_str());
		}

		int num_rounds = 200;
		for (int i = 0; i < num_rounds; ++i) {
			XGBoosterUpdateOneIter(booster, i, dtrain);
		}

		// Run predictions on the test set
		bst_ulong out_len;
		const float* predictions;
		XGBoosterPredict(booster, dtest, 0, 0, 0, &out_len, &predictions);

		// Calculate errors and metrics
		double tp = 0, fp = 0, fn = 0, tn = 0;
		for (size_t i = 0; i < out_len; ++i) {
			float predicted_xg = predictions[i];
			float actual_xg = static_cast<float>(testShots[i].xg);

			float error = predicted_xg - actual_xg;

			float acceptedError = 0.1;

			// If the prediction is within accepted error value of the actual xG, it's a correct prediction (True Positive)
			if (std::abs(error) <= acceptedError) {
				tp++;  // True Positive: Prediction is within the margin of error
			}
			else if (error > acceptedError) {
				fp++;  // False Positive (Type I error): Predicted too high
			}
			else if (error < -acceptedError) {
				fn++;  // False Negative (Type II error): Predicted too low
			}
		}

		// Calculate metrics for this fold
		double accuracy = (tp + tn) / (tp + fp + fn + tn);
		double recall = tp / (tp + fn);
		double precision = tp / (tp + fp);

		total_accuracy += accuracy;
		total_recall += recall;
		total_precision += precision;

		std::vector<double> predictedXG;
		std::vector<double> trueXG;

		for (size_t i = 0; i < out_len; ++i) {
			predictedXG.push_back(predictions[i]);
			trueXG.push_back(testShots[i].xg);  // Extracting actual xG
		}

		// Compute MSE
		double MSE = calculateMSE(predictedXG, trueXG);
		std::cout << "Mean Squared Error: " << MSE << std::endl;


		// debugging
		/* 
		std::cout << "Fold: " << fold << " True Positives: " << tp << std::endl;
		std::cout << "Fold: " << fold << " True Negatives: " << tn << std::endl;
		std::cout << "Fold: " << fold << " False Positives: " << fp << std::endl;
		std::cout << "Fold: " << fold << " False Negatives: " << fn << std::endl;
		*/


		// Clean up
		XGBoosterFree(booster);
		XGDMatrixFree(dtrain);
		XGDMatrixFree(dtest);
	}

	// Print average metrics
	std::cout << "Average Accuracy: " << total_accuracy / k << std::endl;
	std::cout << "Average Recall: " << total_recall / k << std::endl;
	std::cout << "Average Precision: " << total_precision / k << std::endl;
}

DMatrixHandle shotToDMatrix(const Shot& shot) {
	double distance = std::sqrt(std::pow(shot.location.x, 2) + std::pow(shot.location.y, 2));
	std::vector<float> features = {
		static_cast<float>(shot.location.x),
		static_cast<float>(shot.location.y),
		static_cast<float>(distance)
	};

	DMatrixHandle dmatrix;
	int result = XGDMatrixCreateFromMat(features.data(), 1, features.size(), -1, &dmatrix);
	if (result != 0) {
		std::cerr << "Failed to create DMatrix for shot." << std::endl;
		return nullptr;
	}

	return dmatrix;
}


void predictShotxg(Shot& testShot) {
	std::string modelFile = "xgboostModel.json";
	BoosterHandle booster;
	int result = XGBoosterCreate(nullptr, 0, &booster);
	if (result != 0 || XGBoosterLoadModel(booster, modelFile.c_str()) != 0) {
		std::cerr << "Failed to load model from file: " << modelFile << std::endl;
		return;
	}

	DMatrixHandle dmatrix = shotToDMatrix(testShot);
	if (!dmatrix) {
		XGBoosterFree(booster);
		return;
	}

	bst_ulong out_len;
	const float* predictions;
	result = XGBoosterPredict(booster, dmatrix, 0, 0, 0, &out_len, &predictions);
	if (result != 0 || out_len == 0) {
		std::cerr << "Prediction failed." << std::endl;
	}
	else {
		// Assign predicted xG to the shot
		testShot.xg = predictions[0];
	}

	XGBoosterFree(booster);
	XGDMatrixFree(dmatrix);

	std::cout << "Shot Location: (" << testShot.location.x << ", " << testShot.location.y << ")" << std::endl;
	std::cout << "Predicted xG: " << testShot.xg << std::endl;

}


/* int main() {
	bool train = true;
	if (train) {

		std::string filename = "transformed_shots.json";
		std::vector<Shot> shots = parseShotData(filename);

		if (shots.empty()) {
			std::cerr << "No data loaded." << std::endl;
			return -1;
		}

		// Define XGBoost parameters
		std::map<std::string, std::string> params = {
			{"objective", "reg:squarederror"},
			{"eval_metric", "logloss"},
			{"max_depth", "9"},
			{"eta", "0.01"},
			{"min_child_weight", "3"},
			{"colsample_bytree", "0.8"},
			{"scale_pos_weight", "20"}
		};

		// parameter tuning
		bool tuning = false;
		const char* param = "scale_pos_weight";
		if (tuning) {
			std::cout << "Tuning paramter: " << param << std::endl;
		}

		bool useCV = true;
		if (useCV) {
			kFoldCrossValidation(shots, 5, params);
		}
		else {
			trainTestNoCV(shots, params);
		}
	}
	else {

		// giving a prediction 
		bool predict = true;
		Shot testShot;
		testShot.location.x = 40.0;
		testShot.location.y = 2.0;
		testShot.xg = 0.0;
		if (predict) {
			predictShotxg(testShot);
		}
	}

	return 0;
}
*/

