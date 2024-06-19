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
  GIT_REPOSITORY      https://github.com/jspanchu/dawn-temp.git
  GIT_TAG             "cmake-refactor"
  GIT_SUBMODULES      ""
  GIT_SHALLOW         ON)
# Forces dawn to generate static libraries and bundle them all into one giant shared library.
set(BUILD_SHARED_LIBS OFF)
set(DAWN_ENABLE_INSTALL ON)
set(DAWN_FETCH_DEPENDENCIES ON)
# Disable unneeded parts
set(DAWN_USE_GLFW OFF)
set(DAWN_BUILD_SAMPLES OFF)
set(TINT_BUILD_CMD_TOOLS OFF)
set(TINT_BUILD_SAMPLES OFF)
set(TINT_BUILD_DOCS OFF)
set(TINT_BUILD_TESTS OFF)
FetchContent_MakeAvailable(dawn)
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
  add_test(NAME test-simple-cpp COMMAND simple_cpp "$<TARGET_FILE:dawn::webgpu_dawn>")
  add_executable(simple_c tests/simple.c)
  target_link_libraries(simple_c PRIVATE webgpu_headers ${CMAKE_DL_LIBS})
  add_test(NAME test-simple-c COMMAND simple_c "$<TARGET_FILE:dawn::webgpu_dawn>")
endif ()
# Packaging
include(CPack)
