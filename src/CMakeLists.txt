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
file(GLOB_RECURSE MAIN_SOURCES 
    streaming_sound.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/miniaudio/extras/miniaudio_split/miniaudio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/milo/milo.h
)

add_library(streaming_sound SHARED
  ${MAIN_SOURCES}
)

set_target_properties(streaming_sound PROPERTIES
  PUBLIC_HEADER streaming_sound.h
  OUTPUT_NAME "streaming_sound"
)

target_compile_definitions(streaming_sound PRIVATE
  $<$<CONFIG:Debug>:DEBUG_BUILD>
)

target_compile_definitions(streaming_sound PUBLIC DART_SHARED_LIB)

target_compile_definitions(streaming_sound PRIVATE
    # milo
    MILO_DEFAULT_LVL=MILO_LVL_WARN
    # miniaudio
    MA_ENABLE_ONLY_SPECIFIC_BACKENDS
    MA_NO_RESOURCE_MANAGER
    # MA_DEBUG_OUTPUT
)

if(LINUX)
    target_compile_definitions(streaming_sound PUBLIC
        # pulseaudio have bad glitching/noise as stated by miniaudio documentation,
        # it can be fixed on user size, but for simplicity we will just use alsa
        MA_ENABLE_PULSEAUDIO
        MA_ENABLE_JACK
        MA_ENABLE_ALSA
    )
elseif(WIN32)
    target_compile_definitions(streaming_sound PUBLIC
        MA_ENABLE_WASAPI
    )
elseif(ANDROID)
    target_compile_definitions(streaming_sound PUBLIC
        MA_ENABLE_AAUDIO
        # MA_ENABLE_OPENSL
    )
elseif(APPLE)
    target_compile_definitions(streaming_sound PUBLIC
        MA_ENABLE_COREAUDIO
    )
elseif(EMSCRIPTEN)
    target_compile_definitions(streaming_sound PRIVATE
        MA_ENABLE_WEBAUDIO
        MA_ENABLE_AUDIO_WORKLETS
    )
endif()

# if(ANDROID)
#     target_link_libraries(${MAIN_LIB} OpenSLES log)
# endif()

if(NOT MSVC)
    target_compile_options(streaming_sound PRIVATE -W -Wall -Wextra -Wformat -Wno-unused-function)
else()
    target_compile_options(streaming_sound PRIVATE -W3)
endif()

if(NOT MSVC)
    set(COMPILE_AND_LINK_OPTS -O3)
endif()

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(streaming_sound PRIVATE "-Wl,-z,max-page-size=16384")
endif()