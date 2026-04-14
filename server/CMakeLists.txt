set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql Concurrent)

qt_add_executable(ChatRoomServer
    src/main.cpp

    # core
    src/core/TcpServer.h src/core/TcpServer.cpp
    src/core/ClientSession.h src/core/ClientSession.cpp
    src/core/MessageRouter.h src/core/MessageRouter.cpp
    src/core/ConnectionPool.h src/core/ConnectionPool.cpp
    src/core/ThreadPool.h src/core/ThreadPool.cpp
    src/core/SessionManager.h src/core/SessionManager.cpp

    # database
    src/database/DatabasePool.h src/database/DatabasePool.cpp
    src/database/UserRepository.h src/database/UserRepository.cpp
    src/database/MessageRepository.h src/database/MessageRepository.cpp
    src/database/FriendRepository.h src/database/FriendRepository.cpp
    src/database/GroupRepository.h src/database/GroupRepository.cpp
    src/database/OfflineMessageManager.h src/database/OfflineMessageManager.cpp

    # crypto
    src/crypto/AESEncryptor.h src/crypto/AESEncryptor.cpp
    src/crypto/RSAEncryptor.h src/crypto/RSAEncryptor.cpp
    src/crypto/AuthHelper.h src/crypto/AuthHelper.cpp
    src/crypto/PasswordHasher.h src/crypto/PasswordHasher.cpp

    # models
    src/models/User.h src/models/User.cpp
    src/models/Message.h src/models/Message.cpp
    src/models/FriendRelation.h src/models/FriendRelation.cpp
    src/models/Group.h src/models/Group.cpp
    src/models/FriendRequest.h src/models/FriendRequest.cpp
)

target_include_directories(ChatRoomServer PRIVATE src)
target_link_libraries(ChatRoomServer PRIVATE
    Qt6::Core Qt6::Network Qt6::Sql Qt6::Concurrent
)
