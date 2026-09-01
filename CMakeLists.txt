cmake_minimum_required(VERSION 3.22)

# Energy Observatory — evidence and attribution for accelerator energy use.
project(EnergyObservatory
        VERSION 1.0.0
        DESCRIPTION "Evidence and attribution layer for accelerator energy behaviour"
        HOMEPAGE_URL "https://github.com/pngen/Energy-Observatory"
        LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --- Options -----------------------------------------------------------------
option(EOBSV_BUILD_TESTS   "Build Energy Observatory tests"       ON)
option(EOBSV_BUILD_EXAMPLES "Build Energy Observatory examples"   ON)
option(EOBSV_BUILD_SHARED   "Build Energy Observatory as a shared library" OFF)
option(EOBSV_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(EOBSV_ENABLE_SANITIZERS  "Enable sanitizers (debug builds)"  OFF)

# --- Standards ----------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# --- Library -------------------------------------------------------------------
add_library(eobsv src/version.cpp)
add_library(EObservatory::eobsv ALIAS eobsv)

target_compile_features(eobsv PUBLIC cxx_std_20)

target_include_directories(eobsv
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(eobsv PROPERTIES
  POSITION_INDEPENDENT_CODE ON
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

# --- Warnings / hardening ------------------------------------------------------
function(eobsv_apply_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
    if(EOBSV_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
    if(EOBSV_ENABLE_SANITIZERS)
      target_compile_options(${target} PRIVATE /fsanitize=address)
    endif()
  else()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic
      -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual
      -Wnull-dereference)
    if(EOBSV_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
    if(EOBSV_ENABLE_SANITIZERS)
      target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
      target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
  endif()
endfunction()

eobsv_apply_warnings(eobsv)

if(MSVC)
  target_compile_definitions(eobsv PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

# --- Tests ---------------------------------------------------------------------
if(EOBSV_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# --- Examples ------------------------------------------------------------------
if(EOBSV_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# --- Install -------------------------------------------------------------------
install(TARGETS eobsv
  EXPORT EnergyObservatoryTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT EnergyObservatoryTargets
  FILE EnergyObservatoryTargets.cmake
  NAMESPACE EObservatory::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EnergyObservatory)

configure_package_config_file(
  cmake/EnergyObservatoryConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/EnergyObservatoryConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EnergyObservatory)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/EnergyObservatoryConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/EnergyObservatoryConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/EnergyObservatoryConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/EnergyObservatory)

# --- Packaging (CPack) ----------------------------------------------------------
set(CPACK_PACKAGE_NAME "EnergyObservatory")
set(CPACK_PACKAGE_VENDOR "pngen")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Evidence and attribution layer for accelerator energy behaviour")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "pngen93@gmail.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_FILE_NAME "EnergyObservatory-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "TGZ")
endif()
include(CPack)
