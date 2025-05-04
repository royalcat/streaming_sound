# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(streaming_sound_library VERSION 0.0.1 LANGUAGES C)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/../.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()

# Sources
# file(GLOB_RECURSE MAIN_SOURCES 
#    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
#    ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/extras/miniaudio_split/miniaudio.c
#    ${CMAKE_CURRENT_SOURCE_DIR}/milo/milo.h
# )

add_library(streaming_sound SHARED
  "${CMAKE_CURRENT_SOURCE_DIR}/recorder.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/player.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/milo.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/extras/miniaudio_split/miniaudio.c"
)

set_target_properties(streaming_sound PROPERTIES
  PUBLIC_HEADER streaming_sound.h
  OUTPUT_NAME "streaming_sound"
)

target_compile_definitions(streaming_sound PUBLIC DART_SHARED_LIB)

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(streaming_sound PRIVATE "-Wl,-z,max-page-size=16384")
endif()
