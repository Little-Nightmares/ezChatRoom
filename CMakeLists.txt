cmake_minimum_required(VERSION 3.16)
project(ChatRoom VERSION 2.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# When cross-compiling, force use of host Qt6 tools
# MUST be set BEFORE find_package(Qt6)
if(CMAKE_CROSSCOMPILING)
    set(QT_HOST_PATH /usr/lib/x86_64-linux-gnu CACHE PATH "" FORCE)
    set(QT_HOST_PATH_CMAKE_DIR /usr/lib/x86_64-linux-gnu/cmake CACHE PATH "" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql Quick Gui)
set(OPENSSL_ROOT_DIR "D:/PROJECT/CCchatroom/openssl")
set(OPENSSL_INCLUDE_DIR "C:/Program Files/OpenSSL-Win64/include")
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)


