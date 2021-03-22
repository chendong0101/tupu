project (tests)

aux_source_directory(. DIR_LIB_SRCS)

add_executable(${PROJECT_NAME} ${DIR_LIB_SRCS})

add_test(NAME unit_tests COMMAND tests)