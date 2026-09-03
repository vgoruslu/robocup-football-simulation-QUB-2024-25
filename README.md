James Young - 40296786
Final Year Project

# Football Simulator for Robocup Org 2D Soccer Sim

The main team "DonnyDynamos" is based on a team "HELIOS-BASE" designedd by Hidehisa Akiyama and Tomoharu Nakashima. Full Details of their work can be found at https://github.com/helios-base/helios-base.

## Project Installation

There are two parts to the project, the Simulated Football Team and the xG Model.
To Run and Install the football team, "DonnyDynamos" you can follow the instructions found in the README of the HELIOS-BASE team mentioned above.

You will also require installation of the RoboCup Soccersim Software, which can be found here: https://github.com/rcsoccersim

The xG Model contains two files to be run: readFilesANDtransformCoordinates.cpp and xgboostalg.cpp

To run the first file, simply compile and run as any other c++ program:
```
g++ -o readFilesANDtransformCoordinates readFilesANDtransformCoordinates.cpp
./readFilesANDtransformCoordinates
```

To run xgboostalg.cpp you must first install XGBoost. The installation guide can be found here: https://xgboost.readthedocs.io/en/stable/install.html

Then to run xgboostalg.cpp you can run these commands:
```
g++ -o xgboostalg xgboostalg.cpp -I/path/to/xgboost/include -L/path/to/xgboost/lib -lxgboost
export LD_LIBRARY_PATH=/path/to/xgboost/lib:$LD_LIBRARY_PATH
./xgboostalg
```