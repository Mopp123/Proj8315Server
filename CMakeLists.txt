cmake_minimum_required (VERSION 3.5)

set (proj_name "MMOGameServer")

project(${proj_name})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories ("${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/Proj8315Common/src" "/usr/local/include" "/usr/include/postgresql/")

file (
	GLOB source_files
	"${PROJECT_SOURCE_DIR}/Proj8315Common/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/Proj8315Common/src/messages/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/game/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/msgs/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/game/world/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/game/objects/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/game/objects/actions/*.cpp"
	"${PROJECT_SOURCE_DIR}/src/utils/*.cpp"
)


#set (source_files
#	src/Main.cpp
#)


add_executable(${proj_name} ${source_files})

find_library(LIBPQXX pqxx)
message(STATUS "PQXX: ${LIBPQXX}")

find_library(LIBPQ pq)
message(STATUS "LIBPQ: ${LIBPQ}")

#attempt to include and link that "pthread" thing...
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

target_link_libraries(${proj_name} Threads::Threads ${LIBPQXX} ${LIBPQ})
