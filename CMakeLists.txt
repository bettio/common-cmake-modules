project(common-cmake-modules NONE)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
# Version number is incremental (1, 2, 3, 4, 46, 123, 678, etc...)
set(PROJECT_VERSION_FULL 4)

include(GitArchive)

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR cmake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/cmake/CommonModules)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")
set(INSTALL_SOURCES_DIR src/common-cmake-modules-${PROJECT_VERSION_FULL} CACHE PATH "Installation directory for source files needed by the cmake modules")

if(NOT IS_ABSOLUTE ${INSTALL_SOURCES_DIR})
    set(INSTALL_SOURCES_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_SOURCES_DIR}")
endif()

configure_file(BasicFindPackageVersion.cmake.in
               "${CMAKE_CURRENT_BINARY_DIR}/CommonModulesConfigVersion.cmake" @ONLY)
configure_file(SopranoAddStorageDefinition.cmake.in
               "${CMAKE_CURRENT_BINARY_DIR}/SopranoAddStorageDefinition.cmake" @ONLY)

install(FILES BasicFindPackageVersion.cmake.in
              cmake_uninstall.cmake.in
              CommonModulesConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/CommonModulesConfigVersion.cmake
              CompilerWarnings.cmake
              doxygen.cfg.in.example
              Doxygen.cmake
              EnableCoverageReport.cmake
              EnableSlocCount.cmake
              FindCppcheck.cmake
              Findgcovr.cmake
              FindLcov.cmake
              FindQJson.cmake
              Findsloccount.cmake
              GenerateCppcheck.cmake
              GetGitRevisionDescription.cmake
              GetGitRevisionDescription.cmake.in
              GitArchive.cmake
              MacroLogFeature.cmake
              MacroWriteBasicCMakeVersionFile.cmake
              ParseArguments.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/SopranoAddStorageDefinition.cmake
        DESTINATION ${INSTALL_CMAKE_DIR})

file(GLOB sources ${CMAKE_SOURCE_DIR}/src/*)
install(FILES ${sources}
        DESTINATION ${INSTALL_SOURCES_DIR})

# Archive
add_make_dist_target(common-cmake-modules ${PROJECT_VERSION_FULL})
