##https://github.com/nlohmann/json.git
##VERSION 3.9.1 


project(nlohmann_json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(NLOHMANN_JSON_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/nlohmann/")

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${NLOHMANN_JSON_INCLUDE_BUILD_DIR}>)
