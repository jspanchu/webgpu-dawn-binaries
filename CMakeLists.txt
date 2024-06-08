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

# Dawn outputs various library files in a directory structure that mirrors
# it's source tree. While this is convenient to understand provenance,
# having all of them under one roof is quite convenient for packaging.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

# Disable unneeded parts
set(DAWN_BUILD_SAMPLES OFF)
set(DAWN_USE_GLFW OFF)
set(DAWN_FETCH_DEPENDENCIES OFF)
set(TINT_BUILD_CMD_TOOLS OFF)
set(TINT_BUILD_SAMPLES OFF)
set(TINT_BUILD_DOCS OFF)
set(TINT_BUILD_TESTS OFF)

FetchContent_MakeAvailable(dawn)

# Until dawn is able to build shared libraries successfully, we build it by hand.
# Ask dawn to generate static libraries and bundle them all into one giant shared library.
set(BUILD_SHARED_LIBS OFF)
list(APPEND CMAKE_MODULE_PATH "${dawn_SOURCE_DIR}/tools/android/webgpu/src/main/cpp")
include(BundleLibraries)
# So that wgpuXXX symbols are exported from webgpu.h
target_compile_definitions(webgpu_dawn
  PRIVATE "WGPU_IMPLEMENTATION"
  PUBLIC  "WGPU_SHARED_LIBRARY")
bundle_libraries(webgpu_dyndawn webgpu_dawn dawn_native absl::raw_hash_set)
# Handle platform specific link libraries. This was done in dawn/native/CMakeLists.txt,
# however, they're lost since all libraries were shredded down to individial objects in bundle_libraries.
if (DAWN_ENABLE_METAL)
  if (DAWN_TARGET_MACOS)
    target_link_libraries(webgpu_dyndawn PRIVATE "-framework Cocoa")
  endif()
  target_link_libraries(webgpu_dyndawn PRIVATE
  "-framework IOKit"
  "-framework IOSurface"
  "-framework QuartzCore"
  "-framework Metal")
endif ()
if (WIN32 AND NOT WINDOWS_STORE)
  target_link_libraries(webgpu_dyndawn PRIVATE user32.lib)
endif()
if (WINDOWS_STORE)
  target_link_libraries(webgpu_dyndawn PRIVATE debug dxgi.lib)
endif()
if (DAWN_ENABLE_D3D12)
  target_link_libraries(webgpu_dyndawn PRIVATE dxguid.lib)
  if (DAWN_USE_BUILT_DXC)
    target_link_libraries(webgpu_dyndawn PRIVATE dxcompiler)
  endif()
endif ()

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
  target_compile_features(simple_cpp PRIVATE cxx_std_17)
  target_link_libraries(simple_cpp PRIVATE webgpu_headers)
  add_test(NAME test-simple-cpp COMMAND simple_cpp "$<TARGET_FILE:webgpu_dyndawn>")
  add_executable(simple_c tests/simple.c)
  target_link_libraries(simple_c PRIVATE webgpu_headers)
  add_test(NAME test-simple-c COMMAND simple_c "$<TARGET_FILE:webgpu_dyndawn>")
endif ()
