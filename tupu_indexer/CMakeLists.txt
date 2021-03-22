project(tupu_indexer)

cmake_minimum_required(VERSION 3.17)

set(CMAKE_CXX_STANDARD 11)

find_library(KAFKACPP_LIB rdkafka++)
find_library(KAFKA_LIB rdkafka)
find_library(CC_LIB opencc)
find_library(GLOG_LIB glog)
find_library(GFLAGS_LIB gflags)

if (APPLE)
    find_library(PB_LIB protobuf)
    find_library(MONGO_LIB mongocxx)
    find_library(BSON_LIB bsoncxx)
    find_library(ICONV iconv)
    set(THIRDPARTY_LIBS pthread OpenSSL::SSL OpenSSL::Crypto ${ICONV})
else ()
    find_library(PB_LIB protobuf /opt/sogou/protobuf3/root/usr/lib64/)
    find_library(MONGO_LIB mongocxx /opt/rh/rh-mongodb36/root/usr/lib64/)
    find_library(BSON_LIB bsoncxx /opt/rh/rh-mongodb36/root/usr/lib64/)
    find_library(ICONV ticonv)
    set(THIRDPARTY_LIBS ${ICONV})

    set(Protobuf_INCLUDE_DIR /opt/sogou/protobuf3/root/usr/include/)
    set(BSONCXX_INCLUDE_DIR /opt/rh/rh-mongodb36/root/usr/include/bsoncxx/v_noabi)
    set(MONGOCXX_INCLUDE_DIR /opt/rh/rh-mongodb36/root/usr/include/mongocxx/v_noabi)
    set(MONGO_BOOST_INCLUDE_DIR /opt/rh/rh-mongodb36/root/usr/include/)
endif ()

find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(workflow REQUIRED CONFIG HINTS ..)

include_directories(${PROTOBUF_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${WORKFLOW_INCLUDE_DIR} ${BSONCXX_INCLUDE_DIR} ${MONGOCXX_INCLUDE_DIR} ${MONGO_BOOST_INCLUDE_DIR})

set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} workflow ${GLOG_LIB} ${GFLAGS_LIB} ${KAFKA_LIB} ${KAFKACPP_LIB} ${CC_LIB} ${PB_LIB} ${MONGO_LIB} ${BSON_LIB})
link_directories(${WORKFLOW_LIB_DIR})

add_subdirectory(thirdparty/cppjieba)
add_subdirectory(thirdparty/json)
add_subdirectory(src/message)
add_subdirectory(src/utils)
add_subdirectory(src/update_api)
add_subdirectory(src/clientproxy)
add_subdirectory(src/indexer)
add_subdirectory(src/oplog)

set(THIRDPARTY_LIBS ${THIRDPARTY_LIBS} nlohmann_json cppjieba)

aux_source_directory(src SRC_FILES)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} oplog indexer update_api clientproxy utils message ${THIRDPARTY_LIBS})