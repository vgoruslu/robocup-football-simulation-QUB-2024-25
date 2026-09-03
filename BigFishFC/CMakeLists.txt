cmake_minimum_required(VERSION 3.10)
project(Simulator)

set(CMAKE_CXX_STANDARD 17)

# Add source files
add_executable(Simulator
    src/main.cpp
    src/Config.cpp
    src/Player.cpp
    src/Coach.cpp
    src/Simulator.cpp
)

# Include header files
target_include_directories(Simulator PRIVATE include)

# JSON library (if using nlohmann-json)
find_package(nlohmann_json REQUIRED)
target_link_libraries(Simulator PRIVATE nlohmann_json::nlohmann_json)
