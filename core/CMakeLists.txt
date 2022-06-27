cmake_minimum_required(VERSION 3.0)

aux_source_directory(src SRC)

add_library(core ${SRC})
target_include_directories(core PUBLIC include)
target_link_libraries(core utils)
