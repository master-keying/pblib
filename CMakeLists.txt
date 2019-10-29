cmake_minimum_required(VERSION 3.5)
project(PBlib VERSION 1.2.1 LANGUAGES CXX)

# Force C++11, also disable GNU extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# load the MiniSat library
add_subdirectory(cli/minisat EXCLUDE_FROM_ALL)
# load default installation directories
include(GNUInstallDirs)

set(PBLIB_SOURCE_FILES
    pblib/auxvarmanager.cpp
    pblib/clausedatabase.cpp
    pblib/formula.cpp
    pblib/helper.cpp
    pblib/incpbconstraint.cpp
    pblib/IncrementalData.cpp
    pblib/IncSimplePBConstraint.cpp
    pblib/pb2cnf.cpp
    pblib/PBConfig.cpp
    pblib/pbconstraint.cpp
    pblib/PBFuzzer.cpp
    pblib/PBParser.cpp
    pblib/preencoder.cpp
    pblib/SimplePBConstraint.cpp
    pblib/VectorClauseDatabase.cpp
    pblib/encoder/adderencoding.cpp
    pblib/encoder/amo.cpp
    pblib/encoder/bdd.cpp
    pblib/encoder/BDD_Seq_Amo.cpp
    pblib/encoder/bimander_amo_encoding.cpp
    pblib/encoder/binary_amo.cpp
    pblib/encoder/BinaryMerge.cpp
    pblib/encoder/cardencoding.cpp
    pblib/encoder/commander_encoding.cpp
    pblib/encoder/Encoder.cpp
    pblib/encoder/k-product.cpp
    pblib/encoder/naive_amo_encoder.cpp
    pblib/encoder/sorting_merging.cpp
    pblib/encoder/SortingNetworks.cpp
    pblib/encoder/SWC.cpp
)

add_library(pblib ${PBLIB_SOURCE_FILES})
set_target_properties(pblib PROPERTIES OUTPUT_NAME "pb")
add_library(PBlib::pblib ALIAS pblib)

target_include_directories(pblib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# executables
add_executable(pbo2maxsat
    cli/pbo2maxsat.cpp
)
target_link_libraries(pbo2maxsat pblib)

add_executable(pbencoder
    cli/pbencoder.cpp
)
target_link_libraries(pbencoder pblib)

add_executable(pbsolver
    cli/pbsolver.cpp
    cli/BasicSATSolver.cc
    cli/SATSolverClauseDatabase.cpp
)
target_link_libraries(pbsolver MiniSat::libminisat pblib)

add_executable(fuzzer
    cli/fuzzer.cpp
    cli/BasicSATSolver.cc
    cli/SATSolverClauseDatabase.cpp
)
target_link_libraries(fuzzer MiniSat::libminisat pblib)

# installation
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/PseudoBoolean)

install(TARGETS pblib
    EXPORT PBlibTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS pbo2maxsat pbencoder pbsolver
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY pblib
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h*"
)

install(EXPORT PBlibTargets
    FILE PBlibTargets.cmake
    NAMESPACE PBlib::
    DESTINATION ${INSTALL_CONFIGDIR}
)

# ConfigVersion file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/PBlibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/PBlibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/PBlibConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/PBlibConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/PBlibConfig.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

export(EXPORT PBlibTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/PBlibTargets.cmake
    NAMESPACE PBlib::
)

export(PACKAGE PBlib)

function(AddWarnings target)
    if ( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
        target_compile_options( ${target} PRIVATE -Wall -Wextra )
    endif()
    if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
        target_compile_options( ${target} PRIVATE /W4 /wd4800 /wd4200 /wd4245 )
    endif()
endfunction()



AddWarnings(pbo2maxsat)
AddWarnings(pbencoder)
AddWarnings(pbsolver)
AddWarnings(fuzzer)
AddWarnings(pblib)
