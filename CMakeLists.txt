cmake_minimum_required(VERSION 3.10)
project(CS2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(OPENSSL_API_COMPAT=0x10100000L)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(OpenSSL REQUIRED)

set(SOURCE_FILES
    main.cpp              
    Authentication.cpp
    Users.cpp
)

add_executable(test ${SOURCE_FILES})

target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto pthread)
