cmake_minimum_required(VERSION 3.0)

aux_source_directory(src SRC)

add_library(dbg ${SRC} )
target_include_directories(dbg PUBLIC include)
target_link_libraries(dbg ws2_32 wsock32 core)
