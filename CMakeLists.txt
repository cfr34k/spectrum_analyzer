cmake_minimum_required (VERSION 2.6)
project (spectrum_analyzer)

# The current project version
set(VERSION_NUMBER "0.0.0")

# CFLAGS configuration
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-variadic-macros -pedantic -Wno-long-long -D_POSIX_CXX_SOURCE=20120607L -D_FILE_OFFSET_BITS=64")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find Sources
file(GLOB_RECURSE CXX_SRCS src/*.cpp)
file(GLOB_RECURSE CXX_HDRS src/*.h)

# FFTW
find_library(FFTW_LIBRARY fftw3)
set(FFTW_LIBRARIES "${FFTW_LIBRARY}")

# SFML
find_package(SFML COMPONENTS graphics window system REQUIRED)
include_directories("${SFML_INCLUDE_DIR}")

# Version header generation
find_package(Git)

execute_process(COMMAND
  "${GIT_EXECUTABLE}" rev-parse --short HEAD
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	OUTPUT_VARIABLE REVISION
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in" "${CMAKE_CURRENT_BINARY_DIR}/version.h" @ONLY)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Linking
add_executable(${CMAKE_PROJECT_NAME} ${CXX_SRCS})

# additional libraries
target_link_libraries(${CMAKE_PROJECT_NAME} ${FFTW_LIBRARIES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${SFML_LIBRARIES})
