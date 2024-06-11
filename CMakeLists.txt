cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
file(READ "chromium_version.txt" CHROMIUM_VERSION)
string(REPLACE "." ";" VERSION_LIST ${CHROMIUM_VERSION})
list(GET VERSION_LIST 0 CHROMIUM_VERSION_MAJOR)
list(GET VERSION_LIST 1 CHROMIUM_VERSION_MINOR)
list(GET VERSION_LIST 2 CHROMIUM_VERSION_PATCH)
list(GET VERSION_LIST 3 CHROMIUM_VERSION_TWEAK)
project(webgpu-dawn
  VERSION ${CHROMIUM_VERSION_MAJOR}.${CHROMIUM_VERSION_MINOR}.${CHROMIUM_VERSION_PATCH}.${CHROMIUM_VERSION_TWEAK}
  LANGUAGES C CXX)
message(STATUS "Building dawn@chromium/${CHROMIUM_VERSION_PATCH} version ${PROJECT_VERSION}")
# D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR is a temporary workaround for a Visual Studio 2022 and github actions mess
# check if resolved here: https://github.com/actions/runner-images/issues/10004
if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
endif ()
include(FetchContent)
# speed up subsequent configure triggers by not updating git repo
# and therefore avoiding the need to run `gclient sync` during reconfigure
# See https://cmake.org/cmake/help/latest/module/ExternalProject.html#update-step-options
set(dawn_disconnect_subsequent_updates ON)
FetchContent_Declare(dawn
  GIT_REPOSITORY      https://dawn.googlesource.com/dawn
  GIT_TAG             "chromium/${CHROMIUM_VERSION_PATCH}"
  GIT_SUBMODULES      ""
  GIT_SHALLOW         ON
  UPDATE_DISCONNECTED ${dawn_disconnect_subsequent_updates}
  PATCH_COMMAND
    "${CMAKE_COMMAND}"
      -E copy "scripts/standalone.gclient" ".gclient" &&
    "${CMAKE_COMMAND}"
      -V
      -P
      "${CMAKE_CURRENT_LIST_DIR}/tools/cmake/GclientSync.cmake"
      "-DGCLIENT_SYNC_TARGET_SOURCE_DIR=${FETCHCONTENT_BASE_DIR}/dawn-src")
# Dawn outputs various library files in a directory structure that mirrors
# it's source tree. While this is convenient to understand provenance,
# having all of them under one roof is quite convenient for packaging.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
# Forces dawn to generate static libraries and bundle them all into one giant shared library.
set(BUILD_SHARED_LIBS OFF)
# Disable unneeded parts
set(DAWN_BUILD_SAMPLES OFF)
set(DAWN_USE_GLFW OFF)
set(DAWN_FETCH_DEPENDENCIES OFF)
set(TINT_BUILD_CMD_TOOLS OFF)
set(TINT_BUILD_SAMPLES OFF)
set(TINT_BUILD_DOCS OFF)
set(TINT_BUILD_TESTS OFF)
# add dawn source directory to build system
FetchContent_MakeAvailable(dawn)
# This is all of necessary dawn libraries bundled up.
add_library(dawn SHARED "${dawn_BINARY_DIR}/gen/src/dawn/native/webgpu_dawn_native_proc.cpp")
add_library(dawn::dawn ALIAS dawn)
# 'nested-namespace-definition' requires compiler flag '/std:c++17'
target_compile_features(dawn PRIVATE cxx_std_17)
target_link_libraries(dawn
  PRIVATE dawn_native)
# associate the header with the target so that it gets installed.
target_sources(dawn INTERFACE
  FILE_SET dawn_webgpu_header
  TYPE HEADERS
  BASE_DIRS "${dawn_BINARY_DIR}/gen/include"
  FILES "${dawn_BINARY_DIR}/gen/include/dawn/webgpu.h")
target_compile_definitions(dawn
  PUBLIC "WGPU_SHARED_LIBRARY")
target_compile_definitions(dawn
  PRIVATE "WGPU_IMPLEMENTATION")
# installs header `dawn/webgpu.h` and the shared library `dawn`
include(GNUInstallDirs)
install(TARGETS dawn
  EXPORT dawn-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  FILE_SET dawn_webgpu_header)
install(
  DIRECTORY "${dawn_BINARY_DIR}/gen/emscripten-bits"
  TYPE LIB)
# installs helpful package info so that folks can locate us via cmake find_package or pkg-config.  
install(EXPORT dawn-targets
  FILE dawnTargets.cmake
  NAMESPACE dawn::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dawn)
# Create a ConfigVersion.cmake file:
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/dawnConfigVersion.cmake
  COMPATIBILITY AnyNewerVersion)
# Configure config file
configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/dawnConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/dawnConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dawn)
# Install the fully generated config and configVersion files
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/dawnConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/dawnConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dawn)
# Run simple unit tests to ensure wgpu symbols were exported.
include(CTest)
set(BUILD_TESTING ON)
if (BUILD_TESTING)
  set(webgpu_headers_disconnect_subsequent_updates ON)
  FetchContent_Declare(webgpu_headers
    GIT_REPOSITORY "https://github.com/webgpu-native/webgpu-headers.git"
    GIT_TAG         "f686ddfe70bf77a6b0e33a6b7dfce09c93e249c0" # June 10, 2024
    GIT_SUBMODULES  ""
    UPDATE_DISCONNECTED ${webgpu_headers_disconnect_subsequent_updates})
  FetchContent_MakeAvailable(webgpu_headers)
  add_library(webgpu_headers INTERFACE)
  target_include_directories(webgpu_headers INTERFACE "${webgpu_headers_SOURCE_DIR}")

  add_executable(simple_cpp tests/simple.cpp)
  target_compile_features(simple_cpp PRIVATE cxx_std_17)
  target_link_libraries(simple_cpp PRIVATE webgpu_headers ${CMAKE_DL_LIBS})
  add_test(NAME test-simple-cpp COMMAND simple_cpp "$<TARGET_FILE:dawn>")
  add_executable(simple_c tests/simple.c)
  target_link_libraries(simple_c PRIVATE webgpu_headers ${CMAKE_DL_LIBS})
  add_test(NAME test-simple-c COMMAND simple_c "$<TARGET_FILE:dawn>")
endif ()
# Packaging
include(CPack)
