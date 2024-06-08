cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(DynDawn)

include(FetchContent)

# speed up subsequent configures by not updating git repo
# and therefore avoiding the need to run gclient sync.
# See https://cmake.org/cmake/help/latest/module/ExternalProject.html#update-step-options
set(dawn_disconnect_subsequent_updates ON)

FetchContent_Declare(dawn
  GIT_REPOSITORY https://dawn.googlesource.com/dawn
  GIT_TAG        chromium/6526
  GIT_SUBMODULES ""
  GIT_SHALLOW    ON
  UPDATE_DISCONNECTED ${dawn_disconnect_subsequent_updates}
  PATCH_COMMAND
    "${CMAKE_COMMAND}"
      -E copy "scripts/standalone.gclient" ".gclient" &&
    "${CMAKE_COMMAND}"
      -V
      -P
      "${CMAKE_CURRENT_LIST_DIR}/GclientSync.cmake"
      "-DGCLIENT_SYNC_TARGET_SOURCE_DIR=${FETCHCONTENT_BASE_DIR}/dawn-src")

# Disable unneeded parts
set(DAWN_BUILD_SAMPLES OFF)
set(DAWN_USE_GLFW OFF)
set(DAWN_FETCH_DEPENDENCIES OFF)
set(TINT_BUILD_CMD_TOOLS OFF)
set(TINT_BUILD_SAMPLES OFF)
set(TINT_BUILD_DOCS OFF)
set(TINT_BUILD_TESTS OFF)

FetchContent_MakeAvailable(dawn)

# add_library(dawn SHARED "${dawn_BINARY_DIR}/gen/src/dawn/native/webgpu_dawn_native_proc.cpp")
# # 'nested-namespace-definition' requires compiler flag '/std:c++17'
# target_compile_features(dawn PRIVATE cxx_std_17)
# target_link_libraries(dawn
#   PRIVATE dawn_native
#   PUBLIC dawn_headers)
# target_compile_definitions(dawn
#   PUBLIC "WGPU_SHARED_LIBRARY")
# target_compile_definitions(dawn
#   PRIVATE "WGPU_IMPLEMENTATION")

include(CTest)
set(BUILD_TESTING ON)
if (BUILD_TESTING)
  set(webgpu_headers_disconnect_subsequent_updates ON)
  FetchContent_Declare(webgpu_headers
    GIT_REPOSITORY "https://github.com/webgpu-native/webgpu-headers.git"
    GIT_TAG         2a1599527dcacae8af6240c43902ec10b5bb22c3 # June 7, 2024
    GIT_SUBMODULES  ""
    GIT_SHALLOW     ON
    UPDATE_DISCONNECTED ${webgpu_headers_disconnect_subsequent_updates})
  FetchContent_MakeAvailable(webgpu_headers)
  add_library(webgpu_headers INTERFACE)
  target_include_directories(webgpu_headers INTERFACE "${webgpu_headers_SOURCE_DIR}")

  add_executable(simple_cpp tests/simple.cpp)
  target_link_libraries(simple_cpp PRIVATE webgpu_headers)
  add_test(test-simple-cpp simple_cpp)
  add_executable(simple_c tests/simple.c)
  target_link_libraries(simple_c PRIVATE webgpu_headers)
  add_test(test-simple-c simple_c)
endif ()
