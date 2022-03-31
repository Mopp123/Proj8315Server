cmake_minimum_required (VERSION 3.5)

set (proj_name "MMOGameServer")

project(${proj_name})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories ("${PROJECT_SOURCE_DIR}/src")

file (GLOB source_files "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/game/*.cpp" "${PROJECT_SOURCE_DIR}/src/cmds/*.cpp") 


#set (source_files
#	src/Main.cpp
#)

add_executable(${proj_name} ${source_files})


#attempt to include and link that "pthread" thing...
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

target_link_libraries(${proj_name} Threads::Threads)
