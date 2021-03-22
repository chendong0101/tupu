project(tupu_sql_server)

set(CMAKE_INSTALL_PREFIX /opt/sogou CACHE PATH "install prefix")
set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 11)

find_library(CC_LIB opencc)
find_library(GRPC_LIB grpc)
find_library(GRPCPP_LIB grpc++)
find_library(GFLAGS_LIB gflags)
find_library(ACE_LIB ACE)
find_library(OpenSSL openssl)
set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} ${CC_LIB} ${GRPC_LIB} ${GRPCPP_LIB} ${GFLAGS_LIB} ${ACE_LIB})
message(${GRPCPP_LIB})

if (APPLE)
    find_library(PB_LIB protobuf)
    find_library(ICONV iconv)
    set(OPENSSL_INCLUDE_DIR /usr/local/opt/openssl/include)
	set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} OpenSSL::SSL OpenSSL::Crypto ${ICONV} ${PB_LIB})
else ()
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX} "/opt/sogou/protobuf3/root/usr/lib64/")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    find_library(PB_LIB protobuf /opt/sogou/protobuf3/root/usr/lib64/)
    find_library(ICONV ticonv)

    set(Protobuf_INCLUDE_DIR /opt/sogou/protobuf3/root/usr/include/)
	set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} ${ICONV} ${PB_LIB})
endif ()

find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(workflow REQUIRED CONFIG HINTS ..)

include_directories(${PROTOBUF_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${WORKFLOW_INCLUDE_DIR})

set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} workflow)
link_directories(${WORKFLOW_LIB_DIR})

add_subdirectory(thirdparty/json)
add_subdirectory(src/message)
add_subdirectory(src/log)
add_subdirectory(src/utils)
add_subdirectory(src/clientproxy)
add_subdirectory(src/service)
add_subdirectory(src/http)
add_subdirectory(src/grpc)

set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} nlohmann_json)

aux_source_directory(./src SRC_FILES)
message(${SRC_FILES})
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} grpc http service clientproxy utils log message ${THIRDPARTY_LIBS})

install(
	TARGETS ${PROJECT_NAME}
	RUNTIME
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# configure of unit test #
find_library(GTest_LIB gtest)
set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} ${GTest_LIB})
enable_testing()
add_subdirectory(tests)
target_include_directories(tests PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(tests http service clientproxy utils log message ${THIRDPARTY_LIBS})
