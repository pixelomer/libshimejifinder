cmake_minimum_required(VERSION 3.14)
project(shimejifinder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
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

set(BUILD_SHARED_LIBS OFF)
if(NOT TARGET pugixml)
    add_subdirectory(pugixml)
endif()
set(BUILD_SHARED_LIBS ON)

if(NOT DEFINED SHIMEJIFINDER_BUILD_LIBARCHIVE)
    set(SHIMEJIFINDER_BUILD_LIBARCHIVE YES)
endif()

if(NOT DEFINED SHIMEJIFINDER_USE_LIBUNARR)
    set(SHIMEJIFINDER_USE_LIBUNARR YES)
endif()

if (SHIMEJIFINDER_USE_LIBUNARR)
    add_subdirectory(unarr)
endif()
if(SHIMEJIFINDER_BUILD_LIBARCHIVE)
    add_subdirectory(libarchive)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBARCHIVE REQUIRED libarchive)
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
    shimejifinder/extractor.cc
    shimejifinder/fs_extractor.cc
    shimejifinder/memory_extractor.cc
    shimejifinder/utf8_convert/jni.cc
    shimejifinder/utf8_convert/icu.cc
    shimejifinder/utf8_convert/iconv.cc
    shimejifinder/utils.cc
)

if(SHIMEJIFINDER_USE_CUSTOM_UTF8_CONVERT)
    # consumer of libshimejifinder must provide shimejifinder::is_valid_utf8
    # and shimejifinder::shift_jis_to_utf8
    target_compile_definitions(shimejifinder PUBLIC -DSHIMEJIFINDER_USE_CUSTOM_UTF8_CONVERT=1
        -DSHIMEJIFINDER_HAS_UTF8_CONVERT=1)
elseif(SHIMEJIFINDER_USE_ICU)
    find_package(ICU COMPONENTS uc REQUIRED)
    target_link_libraries(shimejifinder ICU::uc)
    target_compile_definitions(shimejifinder PUBLIC -DSHIMEJIFINDER_USE_ICU=1
        -DSHIMEJIFINDER_HAS_UTF8_CONVERT=1)
elseif(SHIMEJIFINDER_USE_ICONV)
    find_package(Iconv)
    target_link_libraries(shimejifinder Iconv::Iconv)
    target_compile_definitions(shimejifinder PUBLIC -DSHIMEJIFINDER_USE_ICONV=1
        -DSHIMEJIFINDER_HAS_UTF8_CONVERT=1)
elseif(SHIMEJIFINDER_USE_JNI)
    find_package(JNI REQUIRED)
    target_link_libraries(shimejifinder JNI::JNI JNI::JVM)
    target_include_directories(shimejifinder PUBLIC ${JNI_INCLUDE_DIRS})
    target_compile_definitions(shimejifinder PUBLIC -DSHIMEJIFINDER_USE_JNI=1
        -DSHIMEJIFINDER_HAS_UTF8_CONVERT=1)
elseif(ANDROID AND (SHIMEJIFINDER_USE_JNI OR NOT DEFINED SHIMEJIFINDER_USE_JNI))
    target_link_libraries(shimejifinder android)
    target_compile_definitions(shimejifinder PUBLIC -DSHIMEJIFINDER_USE_JNI=1
        -DSHIMEJIFINDER_HAS_UTF8_CONVERT=1)
else()
    target_compile_definitions(shimejifinder PUBLIC
        -DSHIMEJIFINDER_HAS_UTF8_CONVERT=0)
endif()

if(CMAKE_BUILD_TYPE EQUAL "Release")
    target_compile_options(shimejifinder PUBLIC -O3 -Wall -Wextra -Werror -Wpedantic)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    target_compile_options(shimejifinder PUBLIC -O0 -g -Wall -Wextra -Werror -Wpedantic)
endif()

add_dependencies(shimejifinder default_xmls_target)
add_dependencies(shimejifinder pugixml)
target_link_libraries(shimejifinder pugixml)
if(SHIMEJIFINDER_USE_LIBUNARR)
    target_link_libraries(shimejifinder unarr)
    add_dependencies(shimejifinder unarr)
else()
    target_compile_definitions(shimejifinder PUBLIC -DSHIMEJIFINDER_NO_LIBUNARR=1)
endif()
if(SHIMEJIFINDER_BUILD_LIBARCHIVE)
    target_link_libraries(shimejifinder archive)
    add_dependencies(shimejifinder archive)
else()
    target_link_libraries(shimejifinder ${LIBARCHIVE_LIBRARIES})
    target_include_directories(shimejifinder PUBLIC ${LIBARCHIVE_INCLUDE_DIRS})
endif()

if(SHIMEJIFINDER_BUILD_EXAMPLES)
    add_executable(shimejifinder-test main.cc)
    target_link_libraries(shimejifinder-test shimejifinder)
endif()
