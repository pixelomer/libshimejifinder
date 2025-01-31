cmake_minimum_required(VERSION 3.14)
project(shimejifinder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(BUILD_SHARED_LIBS ON)
set(ENABLE_UNZIP OFF)
set(ENABLE_TEST OFF)
set(ENABLE_CAT OFF)
set(ENABLE_TAR OFF)
set(ENABLE_CPIO OFF)

add_custom_command(
    OUTPUT default_xmls
    COMMAND ./bin2cpp.sh default_actions.xml shimejifinder/default_actions.cc &&
        ./bin2cpp.sh default_behaviors.xml shimejifinder/default_behaviors.cc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS bin2cpp.sh
)

add_custom_target(
    default_xmls_target
    DEPENDS default_xmls
)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(NOT DEFINED SHIMEJIFINDER_BUILD_LIBARCHIVE)
    set(SHIMEJIFINDER_BUILD_LIBARCHIVE YES)
endif()

add_subdirectory(unarr)
if(SHIMEJIFINDER_BUILD_LIBARCHIVE)
    add_subdirectory(libarchive)
else()
    find_package(LibArchive REQUIRED)
endif()
include_directories(.)

if(NOT DEFINED SHIMEJIFINDER_BUILD_EXAMPLES)
    set(SHIMEJIFINDER_BUILD_EXAMPLES YES)
endif()

add_library(
    shimejifinder STATIC
    shimejifinder/libarchive/archive.cc
    shimejifinder/libunarr/unarr_FILE.c
    shimejifinder/libunarr/archive.cc
    shimejifinder/analyze.cc
    shimejifinder/archive_folder.cc
    shimejifinder/archive.cc
    shimejifinder/archive_entry.cc
    shimejifinder/extract_target.cc
    shimejifinder/utils.cc
)

if(CMAKE_BUILD_TYPE EQUAL "Release")
    target_compile_options(shimejifinder PUBLIC -O3 -Wall -Wextra -Werror -Wpedantic)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    target_compile_options(shimejifinder PUBLIC -O0 -g -Wall -Wextra -Werror -Wpedantic)
endif()

target_link_libraries(shimejifinder unarr archive)
add_dependencies(shimejifinder unarr default_xmls_target)
if(SHIMEJIFINDER_BUILD_LIBARCHIVE)
    add_dependencies(shimejifinder archive)
endif()

if(SHIMEJIFINDER_BUILD_EXAMPLES)
    add_executable(shimejifinder-test main.cc)
    target_link_libraries(shimejifinder-test shimejifinder)
endif()