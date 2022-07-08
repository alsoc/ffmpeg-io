cmake_minimum_required(VERSION 3.2)

project(ffmpeg-io)

# CMake options ---------------------------------------------------------------
# -----------------------------------------------------------------------------
option(FFMPEG-IO_EXE "Compile the ffmpeg-io executable." ON)

# Print CMake options values --------------------------------------------------
# -----------------------------------------------------------------------------
message(STATUS "FFmpeg-IO options: ")
message(STATUS "  * FFMPEG-IO_EXE: '${FFMPEG-IO_EXE}'")

# Define CMake variables ------------------------------------------------------
# -----------------------------------------------------------------------------
set(src_dir src)
set(inc_dir include)
set(lib_dir lib)
set(exe_dir exe)

# Declare source files to compile ---------------------------------------------
# -----------------------------------------------------------------------------
set(src_files
	${src_dir}/cmd.c
	${src_dir}/common.c
	${src_dir}/probe.c
	${src_dir}/reader.c
	${src_dir}/player.c
	${src_dir}/writer.c
	${src_dir}/formatter.c)

# Compiler generic options ----------------------------------------------------
# -----------------------------------------------------------------------------
# enable C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Specify the library paths ---------------------------------------------------
# -----------------------------------------------------------------------------
set(EXECUTABLE_OUTPUT_PATH ${exe_dir})
set(LIBRARY_OUTPUT_PATH ${lib_dir})

# Create the library ----------------------------------------------------------
# -----------------------------------------------------------------------------
# objects
add_library(ffmpeg-io-obj OBJECT ${src_files})
set_target_properties(ffmpeg-io-obj PROPERTIES POSITION_INDEPENDENT_CODE ON) # set -fpic
# library
add_library(ffmpeg-io-slib STATIC $<TARGET_OBJECTS:ffmpeg-io-obj>)
set_target_properties(ffmpeg-io-slib PROPERTIES OUTPUT_NAME ffmpeg-io POSITION_INDEPENDENT_CODE ON) # set -fpic
# executable
if (FFMPEG-IO_EXE)
	add_executable(ffmpeg-io-exe ${src_dir}/main.c $<TARGET_OBJECTS:ffmpeg-io-obj>)
	set_target_properties(ffmpeg-io-exe PROPERTIES OUTPUT_NAME ffmpeg-io-test POSITION_INDEPENDENT_CODE ON) # set -fpie
endif()

# Set include directory -------------------------------------------------------
# -----------------------------------------------------------------------------
target_include_directories(ffmpeg-io-obj PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${inc_dir}/>)
target_include_directories(ffmpeg-io-slib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${inc_dir}/>)
if (FFMPEG-IO_EXE)
	target_include_directories(ffmpeg-io-exe PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${inc_dir}/>)
endif()

# Export target ---------------------------------------------------------------
# -----------------------------------------------------------------------------
export(TARGETS ffmpeg-io-slib NAMESPACE ffmpeg-io:: FILE "${lib_dir}/cmake/ffmpeg-io/ffmpeg-io-config.cmake")
