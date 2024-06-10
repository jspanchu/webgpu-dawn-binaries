# Create a tag by running cmake -P ./tools/cmake/TagRelease.cmake
find_package(Git)
if(NOT Git_FOUND)
  message("Git executable was not found!")
endif()

file(READ "chromium_version.txt" CHROMIUM_VERSION)
execute_process(
    COMMAND
      "${GIT_EXECUTABLE}" "tag" "-a" "v${CHROMIUM_VERSION}" "-m" "webgpu-dawn-v${CHROMIUM_VERSION}"
    WORKING_DIRECTORY "."
    COMMAND_ECHO STDOUT
    RESULT_VARIABLE res
    ERROR_VARIABLE err
    ERROR_STRIP_TRAILING_WHITESPACE)
if (res)
    message(FATAL_ERROR "Failed to create a tag: ${err}")
endif ()
