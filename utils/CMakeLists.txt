cmake_minimum_required(VERSION 3.0)

aux_source_directory(src SRC)

add_library(utils ${SRC})
target_include_directories(utils PUBLIC include)