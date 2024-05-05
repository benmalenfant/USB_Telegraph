cmake_minimum_required(VERSION 3.0)
project(MyServer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Define the sources
set(SRV_SOURCES
    src/Server.cpp
)

set(CLIENT_SOURCES
    src/Client.cpp
)

# Add executable target
add_executable(server ${SRV_SOURCES})

add_executable(client ${CLIENT_SOURCES})

set(EXECUTABLE_OUTPUT_PATH  ../bin/)

target_link_libraries(server wsock32 ws2_32)

target_link_libraries(client wsock32 ws2_32)