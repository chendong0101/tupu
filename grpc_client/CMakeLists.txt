project(tupu_sql_server)

set(CMAKE_INSTALL_PREFIX /opt/sogou CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 11)

find_library(GRPC_LIB grpc)
find_library(GRPCPP_LIB grpc++)
find_library(GFLAGS_LIB gflags)
find_library(PB_LIB protobuf)
set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} ${GRPC_LIB} ${GRPCPP_LIB} ${GFLAGS_LIB} ${PB_LIB})

find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})

add_subdirectory(src/message)

aux_source_directory(./src SRC_FILES)
message(${SRC_FILES})
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} message ${THIRDPARTY_LIBS})
