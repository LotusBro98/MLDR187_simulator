cmake_minimum_required(VERSION 3.20)
project(MLDR187_simulator)
set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_subdirectory(core)
add_subdirectory(debug)

aux_source_directory(src SRC)

add_executable(MLDR187_simulator ${SRC})
target_link_libraries(MLDR187_simulator core dbg)
