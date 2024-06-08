list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
include(DepotTools)

set(depot_tools_cmake_path "${CMAKE_CURRENT_LIST_DIR}/depot_tools")
cmake_path(NATIVE_PATH depot_tools_cmake_path depot_tools_path)

if (CMAKE_HOST_WIN32)
  set(ENV{DEPOT_TOOLS_WIN_TOOLCHAIN} 0)
  set(ENV{PATH} "${depot_tools_path};$ENV{PATH}")
  execute_process(
    COMMAND
      "cmd.exe" "/C" "gclient" "sync" "-D"
    COMMAND_ECHO STDOUT
    WORKING_DIRECTORY "${GCLIENT_SYNC_TARGET_SOURCE_DIR}"
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
else ()
  set(ENV{PATH} "${depot_tools_path}:$ENV{PATH}")
  execute_process(
    COMMAND
      "gclient" "sync" "-D"
    COMMAND_ECHO STDOUT
    WORKING_DIRECTORY "${GCLIENT_SYNC_TARGET_SOURCE_DIR}"
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
endif ()

if (res)
  message(FATAL_ERROR "gclient sync -D failed ${res}")
endif ()
