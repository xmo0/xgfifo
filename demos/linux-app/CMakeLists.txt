cmake_minimum_required(VERSION 3.10)

############################################################
# Create a library
############################################################

#Generate the static library from the library sources
add_library(xgfifolib STATIC 
    ${PROJECT_SOURCE_DIR}/xgfifo/xgfifo.c
)

target_include_directories(xgfifolib INTERFACE
    ${PROJECT_SOURCE_DIR}/xgfifo
)

############################################################
# Create an executable
############################################################

# Add an executable with the above sources
add_executable(test_simple test_simple.c)
add_executable(test_block test_block.c)
add_executable(test_mtSimple test_mtSimple.c)
add_executable(test_mtBlock test_mtBlock.c)

# link the new hello_library target with the hello_binary target
target_link_libraries(test_simple PRIVATE xgfifolib)
target_link_libraries(test_block PRIVATE xgfifolib)
target_link_libraries(test_mtSimple PRIVATE xgfifolib)
target_link_libraries(test_mtBlock PRIVATE xgfifolib)
