project(basicpbsolver)


cmake_minimum_required(VERSION 2.8)

# SET(CMAKE_CXX_COMPILER "clang++")
SET(CMAKE_CXX_COMPILER "g++")

# set(CMAKE_CXX_FLAGS "-O3 -g3 -std=c++11")
set(CMAKE_CXX_FLAGS "-O3 -std=c++11 -static -Wno-unused-result")

# set(CMAKE_CXX_FLAGS "-O0 -g3 -std=c++11 -static")
# set(CMAKE_CXX_FLAGS "-O1 -g3 -pg -std=c++11")

# set (CMAKE_BUILD_TYPE "Debug")
set (CMAKE_BUILD_TYPE "Release")


include_directories(${basicpbsolver_SOURCE_DIR}/minisat)
include_directories(${basicpbsolver_SOURCE_DIR}/../lib/)
LINK_DIRECTORIES(${basicpbsolver_SOURCE_DIR}/../lib)
LINK_DIRECTORIES(${basicpbsolver_SOURCE_DIR}/minisat)


add_executable(pbsolver main.cpp SATSolverClauseDatabase.cpp)

# this make sure that every time the external libaries are linked again
ADD_CUSTOM_TARGET(CheckForLibs ALL
   COMMAND touch ${basicpbsolver_SOURCE_DIR}/main.cpp
)

add_dependencies(pbsolver CheckForLibs)


TARGET_LINK_LIBRARIES(pbsolver pblib)
TARGET_LINK_LIBRARIES(pbsolver minisat)


install(TARGETS pbsolver RUNTIME DESTINATION bin)
