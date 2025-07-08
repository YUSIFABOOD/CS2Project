cmake_minimum_required(VERSION 3.10)
project(CS2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: suppress OpenSSL 3.0 SHA warnings
add_compile_definitions(OPENSSL_API_COMPAT=0x10100000L)

# If your .h files are in "include/", include it:
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add OpenSSL
find_package(OpenSSL REQUIRED)

set(SOURCE_FILES
    main.cpp              # ✅ This must match the actual file name
    Authentication.cpp
    Users.cpp
)

add_executable(test ${SOURCE_FILES})

target_link_libraries(test OpenSSL::SSL OpenSSL::Crypto pthread)
