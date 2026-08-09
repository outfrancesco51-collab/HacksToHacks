cmake_minimum_required(VERSION 3.11)
project(HacksToHacks C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

add_executable(HacksToHacks
    src/main.c
    src/cJSON.c
    src/tween.c
)

target_include_directories(HacksToHacks PRIVATE src)
target_link_libraries(HacksToHacks PRIVATE raylib)

if(ANDROID)
    target_link_libraries(HacksToHacks PRIVATE log android EGL GLESv2 OpenSLES m)
endif()

file(COPY "/assets" DESTINATION "")
file(COPY "/locales" DESTINATION "")
