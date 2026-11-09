cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(RealMouseClick VERSION 1.1.5)

# The original project stores the WAV under "recources" (legacy typo).
# Copy that same file into Geode's expected "resources" directory during CI;
# this does not replace or modify the user's audio file.
set(CLICK_WAV_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/recources/audio/click.wav")
set(CLICK_WAV_DEST "${CMAKE_CURRENT_SOURCE_DIR}/resources/audio/click.wav")
if (EXISTS "${CLICK_WAV_SOURCE}")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources/audio")
    configure_file("${CLICK_WAV_SOURCE}" "${CLICK_WAV_DEST}" COPYONLY)
endif()


if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK. Set GEODE_SDK to the SDK directory.")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})
