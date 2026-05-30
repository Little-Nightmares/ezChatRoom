set(SERVER_SOURCES
    main.cpp
    src/core/TcpServer.h
    src/core/TcpServer.cpp
    src/core/ClientSession.h
    src/core/ClientSession.cpp
    src/core/MessageRouter.h
    src/core/MessageRouter.cpp
    src/database/ServerDatabase.h
    src/database/ServerDatabase.cpp
    src/crypto/PasswordHasher.h
    src/crypto/PasswordHasher.cpp
    src/admin/AdminHttpServer.h
    src/admin/AdminHttpServer.cpp
    src/core/BotService.h
    src/core/BotService.cpp
    src/admin/admin.qrc
)

qt_add_executable(ChatRoomServer ${SERVER_SOURCES})

target_include_directories(ChatRoomServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/common/src
)

target_link_libraries(ChatRoomServer PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Sql
    CommonLib
)

# Windows system libraries are linked via CommonLib -> OpenSSL::Crypto INTERFACE

# Disable zstd compression in Qt resources (not available in cross-compilation)
target_compile_definitions(ChatRoomServer PRIVATE QT_NO_RESOURCE_ZSTD_COMPRESSION)
