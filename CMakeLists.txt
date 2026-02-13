cmake_minimum_required(VERSION 3.30)
project(chooseprio LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
set(REPROC++ ON CACHE BOOL "Enable reproc C++ library" FORCE)
FetchContent_Declare(
        reproc
        GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
        GIT_TAG        v14.2.4
)
FetchContent_MakeAvailable(reproc)

add_executable(chooseprio_latency chooseprio_latency.cpp)
target_link_libraries(chooseprio_latency PRIVATE reproc++ reproc)
if(WIN32)
    target_link_libraries(chooseprio_latency PRIVATE winmm)
endif()


add_executable(chooseprio_fighter chooseprio_fighter.cpp)
target_link_libraries(chooseprio_fighter PRIVATE reproc++ reproc)
