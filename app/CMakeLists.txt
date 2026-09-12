cmake_minimum_required(VERSION 3.15)

project(telemetry_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(telemetry_app
    src/main.cpp
    src/udp.cpp
    src/telemetry.cpp
)

target_include_directories(telemetry_app PRIVATE
    src
    ../common
)

target_link_libraries(telemetry_app PRIVATE
    ws2_32
)