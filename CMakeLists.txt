cmake_minimum_required(VERSION 3.10)
project(CS2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(OPENSSL_API_COMPAT=0x10100000L)

# Include Crow and Asio headers
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/asio)

# Find and link OpenSSL
find_package(OpenSSL REQUIRED)

# Source files
set(SOURCE_FILES
    main.cpp
    Authentication.cpp
    Users.cpp
)

# Executable
add_executable(test ${SOURCE_FILES})

# Link OpenSSL, pthread, and Windows networking lib
target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto pthread ws2_32 mswsock)

