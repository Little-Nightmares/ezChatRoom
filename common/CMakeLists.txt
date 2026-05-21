cmake_minimum_required(VERSION 3.16)
project(CommonLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(OpenSSL REQUIRED COMPONENTS Crypto)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(COMMON_SOURCES
    src/protocol/ChatProtocol.cpp
    src/crypto/CryptoUtils.cpp
)

set(COMMON_HEADERS
    src/protocol/MessageTypes.h
    src/protocol/ChatProtocol.h
    src/models/ChatMessage.h
    src/models/UserInfo.h
    src/models/FriendRequest.h
    src/crypto/CryptoUtils.h
)

add_library(CommonLib STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

target_include_directories(CommonLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(CommonLib PUBLIC
    Qt6::Core
)
if(TARGET OpenSSL::Crypto)
    target_link_libraries(CommonLib PUBLIC OpenSSL::Crypto)
endif()

# OpenSSL static lib on Windows needs system libs AFTER it.
# Use INTERFACE link options on OpenSSL::Crypto to ensure correct order.
if(WIN32 AND TARGET OpenSSL::Crypto)
    set_property(TARGET OpenSSL::Crypto APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES "ws2_32;crypt32;secur32;bcrypt;advapi32"
    )
endif()
