cmake_minimum_required(VERSION 3.20)
project(MLDR187_simulator)
set(CMAKE_CXX_STANDARD 14)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-DDEBUG)
else()
    add_compile_options(-O3)
endif()

add_subdirectory(utils)
add_subdirectory(core)
add_subdirectory(modules)

set(SOURCES main.cpp )#core/src/init.cpp)
#
#foreach(dir ${MOD_DIR_LIST})
#    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/modules/${dir}/src/init.cpp")
#        set(SOURCES modules/${dir}/src/init.cpp ${SOURCES})
#    endif ()
#endforeach()

add_executable(MLDR187_simulator ${SOURCES})
target_link_libraries(MLDR187_simulator core ${MOD_LIST})
