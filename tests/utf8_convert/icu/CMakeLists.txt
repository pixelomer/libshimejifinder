cmake_minimum_required(VERSION 3.14)
project(icu_test)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(SHIMEJIFINDER_BUILD_EXAMPLES NO)
set(SHIMEJIFINDER_BUILD_LIBARCHIVE NO)
set(SHIMEJIFINDER_USE_LIBUNARR NO)
set(SHIMEJIFINDER_USE_ICU YES)
add_subdirectory(../../.. shimejifinder)
include_directories(../../..)

add_executable(icu_test main.cc ../tests.cc)
target_link_libraries(icu_test shimejifinder gtest)
