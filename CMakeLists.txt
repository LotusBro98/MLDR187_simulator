cmake_minimum_required(VERSION 3.20)
project(MLDR187_simulator)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(core)

include_directories(include)
aux_source_directory(src SRC)

add_executable(MLDR187_simulator ${SRC})
target_link_libraries(MLDR187_simulator core)
