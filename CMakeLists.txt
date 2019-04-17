cmake_minimum_required (VERSION 2.6)
project (net)
 
option (NET_BUILD_EXAMPLES "Build net examples." ON)

set(CMAKE_CXX_STANDARD 17)

include_directories ("include/")

add_library (net STATIC include/litianqi/net.hpp src/net.cpp)

if (NET_BUILD_EXAMPLES)
    add_subdirectory (example)
endif ()