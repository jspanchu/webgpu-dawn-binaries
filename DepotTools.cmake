cmake_minimum_required(VERSION 3.12)

set(depot_tools_giturl "https://chromium.googlesource.com/chromium/tools/depot_tools.git")
set(depot_tools_zipurl "https://storage.googleapis.com/chrome-infra")
set(depot_tools_file "depot_tools.zip")

if (CMAKE_HOST_WIN32)
  # Download depot_tools.zip
  file(DOWNLOAD
    "${depot_tools_zipurl}/${depot_tools_file}"
    "${CMAKE_CURRENT_LIST_DIR}/${depot_tools_file}"
    STATUS download_status)
  # Check the download status.
  list(GET download_status 0 res)
  if (res)
    list(GET download_status 1 err)
    message(FATAL_ERROR
      "Failed to download ${depot_tools_file}: ${err}")
  endif ()
  # Extract the file.
  execute_process(
    COMMAND
      "${CMAKE_COMMAND}"
      -E rm
      -rf "${CMAKE_CURRENT_LIST_DIR}/depot_tools")
  execute_process(
    COMMAND
      "${CMAKE_COMMAND}"
      -E make_directory
      "${CMAKE_CURRENT_LIST_DIR}/depot_tools")
  execute_process(
    COMMAND
      "${CMAKE_COMMAND}"
      -E chdir "${CMAKE_CURRENT_LIST_DIR}/depot_tools" tar
      -xf "${CMAKE_CURRENT_LIST_DIR}/${depot_tools_file}"
    COMMAND_ECHO STDOUT
    WORKING_DIRECTORY "."
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(FATAL_ERROR
      "Failed to extract ${depot_tools_file}: ${err}")
  endif ()
else ()
  find_package(Git)
  if(NOT Git_FOUND)
    message("Git executable was not found!")
  endif()
  execute_process(
    COMMAND
      "${CMAKE_COMMAND}"
      -E rm
      -rf "${CMAKE_CURRENT_LIST_DIR}/depot_tools")
  execute_process(
    COMMAND
      "${GIT_EXECUTABLE}" "clone" "${depot_tools_giturl}" "${CMAKE_CURRENT_LIST_DIR}/depot_tools"
    WORKING_DIRECTORY "."
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(FATAL_ERROR "Failed to clone depot_tools: ${err}")
  endif ()
endif ()
