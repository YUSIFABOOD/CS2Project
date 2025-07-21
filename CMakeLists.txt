cmake_minimum_required(VERSION 3.10)
project(SocialTimeline)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(jsoncpp REQUIRED)

# Add source files
set(SOURCES
    main.cpp
    Authentication.cpp
    Users.cpp
    timeline.cpp
    FriendsManager.cpp
    UserSearchBST.cpp
)

# Add header files
set(HEADERS
    include/Authentication.h
    include/Users.h
    include/timeline.h
    include/FriendsManager.h
    include/AVLTree.h
    include/UserSearchBST.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENSSL_LIBRARIES}
    jsoncpp
    pthread
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
