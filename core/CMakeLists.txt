cmake_minimum_required(VERSION 3.0)

add_library(intelhex libintelhex/src/intelhex.cc)
target_include_directories(intelhex PUBLIC libintelhex/include)

aux_source_directory(src SRC)

add_library(core ${SRC})
target_include_directories(core PUBLIC include)
target_link_libraries(core intelhex)
