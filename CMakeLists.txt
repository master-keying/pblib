cmake_minimum_required(VERSION 3.5)
project(PBlib VERSION 1.2.1 LANGUAGES CXX)

# options
option(PBLIB_OSX_VIRTUAL_DESTRUCTOR_WORKAROUND
    "Hotfix for an older versions of Clang on OS-X" OFF)

# Force C++11, also disable GNU extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# load the MiniSat library
add_subdirectory(BasicPBSolver/minisat EXCLUDE_FROM_ALL)
# load default installation directories
include(GNUInstallDirs)

# pblib
set(PBLIB_HEADER_FILES
    auxvarmanager.h
    clausedatabase.h
    formula.h
    helper.h
    incpbconstraint.h
    IncrementalData.h
    IncSimplePBConstraint.h
    pb2cnf.h
    PBConfig.h
    pbconstraint.h
    PBFuzzer.h
    PBParser.h
    PBSatSolver.h
    preencoder.h
    SimplePBConstraint.h
    VectorClauseDatabase.h
    weightedlit.h
    encoder/adderencoding.h
    encoder/amo.h
    encoder/bdd.h
    encoder/BDD_Seq_Amo.h
    encoder/bimander_amo_encoding.h
    encoder/binary_amo.h
    encoder/BinaryMerge.h
    encoder/cardencoding.h
    encoder/commander_encoding.h
    encoder/Encoder.h
    encoder/k-product.h
    encoder/naive_amo_encoder.h
    encoder/sorting_merging.h
    encoder/SortingNetworks.h
    encoder/SWC.h
)
set(PBLIB_SOURCE_FILES
    auxvarmanager.cpp
    clausedatabase.cpp
    formula.cpp
    helper.cpp
    hotfixes.cpp
    incpbconstraint.cpp
    IncrementalData.cpp
    IncSimplePBConstraint.cpp
    pb2cnf.cpp
    PBConfig.cpp
    pbconstraint.cpp
    PBFuzzer.cpp
    pbo2maxsat.cpp
    PBParser.cpp
    preencoder.cpp
    SimplePBConstraint.cpp
    VectorClauseDatabase.cpp
    encoder/adderencoding.cpp
    encoder/amo.cpp
    encoder/bdd.cpp
    encoder/BDD_Seq_Amo.cpp
    encoder/bimander_amo_encoding.cpp
    encoder/binary_amo.cpp
    encoder/BinaryMerge.cpp
    encoder/cardencoding.cpp
    encoder/commander_encoding.cpp
    encoder/Encoder.cpp
    encoder/k-product.cpp
    encoder/naive_amo_encoder.cpp
    encoder/sorting_merging.cpp
    encoder/SortingNetworks.cpp
    encoder/SWC.cpp
)

add_library(pblib ${PBLIB_SOURCE_FILES})
set_target_properties(pblib PROPERTIES OUTPUT_NAME "pb")
add_library(PBlib::pblib ALIAS pblib)

target_include_directories(pblib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(PBLIB_OSX_VIRTUAL_DESTRUCTOR_WORKAROUND)
    target_compile_definitions( pblib PRIVATE PBLIB_OSX_VIRTUAL_DESTRUCTOR_WORKAROUND )
endif()

# executables
add_executable(pbo2maxsat pbo2maxsat.cpp)
target_link_libraries(pbo2maxsat pblib)

add_executable(pbencoder pbencoder.cpp)
target_link_libraries(pbencoder pblib)

add_executable(pbsolver
    BasicPBSolver/main.cpp
    BasicPBSolver/BasicSATSolver.cc
    BasicPBSolver/SATSolverClauseDatabase.cpp
)
target_link_libraries(pbsolver MiniSat::libminisat pblib)

add_executable(fuzzer fuzzer.cpp
    BasicPBSolver/BasicSATSolver.cc
    BasicPBSolver/SATSolverClauseDatabase.cpp
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

install(FILES ${PBLIB_HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/pblib
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
