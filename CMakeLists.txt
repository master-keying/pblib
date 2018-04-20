PROJECT(pblib)

cmake_minimum_required(VERSION 2.8)

# SET (CMAKE_CXX_COMPILER "clang++")
 SET(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_FLAGS "-O3 -std=c++11")
# set(CMAKE_CXX_FLAGS "-O3 -std=c++11 -static")
# set(CMAKE_CXX_FLAGS "-O3 -g3 -std=c++11")
# set(CMAKE_CXX_FLAGS "-O1 -g3 -std=c++11")
# set(CMAKE_CXX_FLAGS "-O3 -g3 -std=c++0x")

# set (CMAKE_BUILD_TYPE "Debug")
 set (CMAKE_BUILD_TYPE "Release")

ADD_CUSTOM_TARGET(cpfiles ALL
   COMMAND mkdir -p lib/encoder &&  cp *.h lib/ && cp encoder/*.h lib/encoder/ && cp libpblib.* lib/
)


file(GLOB pblib_src_files
    "*.cpp", "encoder/*.cpp"
)

list(REMOVE_ITEM  pblib_src_files "${pblib_SOURCE_DIR}/fuzzer.cpp" "${pblib_SOURCE_DIR}/pbencoder.cpp")


add_library(pblib ${pblib_src_files})
# add_library(pblib SHARED ${pblib_src_files})


# include_directories(${pblib_SOURCE_DIR}/BasicPBSolver/minisat/minisat/)
include_directories(${pblib_SOURCE_DIR}/BasicPBSolver/minisat/)
link_directories(${pblib_SOURCE_DIR}/BasicPBSolver/minisat/)


add_executable(fuzzer EXCLUDE_FROM_ALL fuzzer.cpp BasicPBSolver/SATSolverClauseDatabase.cpp)
target_link_libraries(fuzzer minisat z pblib pthread)

add_executable(pbo2maxsat EXCLUDE_FROM_ALL pbo2maxsat.cpp)
target_link_libraries(pbo2maxsat pblib)

add_executable(pbencoder EXCLUDE_FROM_ALL pbencoder.cpp)
target_link_libraries(pbencoder pblib)

add_custom_target(setup COMMAND
    COMMAND cd BasicPBSolver/ && tar xzf minisat.tgz && cd minisat && cmake . && make -j3 && cd ../.. && cmake . && make -j3 && make fuzzer && make pbo2maxsat && make pbencoder && cd BasicPBSolver/ && cmake . && make -j3
)

add_custom_target(tar COMMAND
    ${CMAKE_COMMAND} -E tar "czf" "pblib.tgz"  "pbo2maxsat.sh" "*.h" "*.cpp" "encoder/*.h" "encoder/*.cpp" "BasicPBSolver/BasicSATSolver.h" "BasicPBSolver/main.cpp" "BasicPBSolver/SATSolverClauseDatabase.cpp" "BasicPBSolver/SATSolverClauseDatabase.h" "CMakeLists.txt" "BasicPBSolver/CMakeLists.txt" "BasicPBSolver/minisat.tgz" "manual/pblib.pdf" "VERSION" "LICENSE" "INSTALL"
)

add_custom_target(tar_lib COMMAND
    ${CMAKE_COMMAND} -E tar "czf" "pblib_static.tgz"  "libpblib.a" "lib/" "manual/pblib.pdf" "LICENSE" "pbencoder"  "manual/pblib.pdf" "BasicPBSolver/pbsolver" "VERSION" 
)


add_dependencies(pbencoder pblib)
add_dependencies(fuzzer pblib)
add_dependencies(cpfiles pblib)
add_dependencies(tar_lib pblib cpfiles)
