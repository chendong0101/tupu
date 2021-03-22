project (utils)

include_directories(${CMAKE_SOURCE_DIR})

aux_source_directory(. DIR_LIB_SRCS)

add_library(${PROJECT_NAME} ${DIR_LIB_SRCS})

target_link_libraries(${PROJECT_NAME} message)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
