##https://github.com/yanyiwu/cppjieba.git
##commit b799936d2f3fa40b43ab7ce1a65481fcb1269a9a

project(cppjieba LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(NLOHMANN_JSON_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include/")

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE 
            $<BUILD_INTERFACE:${NLOHMANN_JSON_INCLUDE_BUILD_DIR}>)
