cmake_minimum_required(VERSION 3.16)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Network Sql)

qt_add_executable(ChatRoomClient
    src/main.cpp

    # core
    src/core/AppManager.h src/core/AppManager.cpp
    src/core/SessionManager.h src/core/SessionManager.cpp
    src/core/NavigationController.h src/core/NavigationController.cpp
    src/core/SettingsManager.h src/core/SettingsManager.cpp

    # network
    src/network/TcpClient.h src/network/TcpClient.cpp
    src/network/MessageHandler.h src/network/MessageHandler.cpp
    src/network/PacketBuilder.h src/network/PacketBuilder.cpp
    src/network/HeartbeatManager.h src/network/HeartbeatManager.cpp

    # crypto
    src/crypto/AESEncryptor.h src/crypto/AESEncryptor.cpp
    src/crypto/RSAEncryptor.h src/crypto/RSAEncryptor.cpp
    src/crypto/HashHelper.h src/crypto/HashHelper.cpp
    src/crypto/KeyManager.h src/crypto/KeyManager.cpp

    # database
    src/database/DatabaseManager.h src/database/DatabaseManager.cpp
    src/database/MessageDAO.h src/database/MessageDAO.cpp
    src/database/UserDAO.h src/database/UserDAO.cpp
    src/database/ConversationDAO.h src/database/ConversationDAO.cpp

    # models
    src/models/UserModel.h src/models/UserModel.cpp
    src/models/MessageModel.h src/models/MessageModel.cpp
    src/models/ConversationModel.h src/models/ConversationModel.cpp
    src/models/FriendRequestModel.h src/models/FriendRequestModel.cpp

    # qmlbridge
    src/qmlbridge/AppCore.h src/qmlbridge/AppCore.cpp
    src/qmlbridge/ChatController.h src/qmlbridge/ChatController.cpp
    src/qmlbridge/UserController.h src/qmlbridge/UserController.cpp
    src/qmlbridge/FriendController.h src/qmlbridge/FriendController.cpp
    src/qmlbridge/FileTransferController.h src/qmlbridge/FileTransferController.cpp
    src/qmlbridge/NotificationController.h src/qmlbridge/NotificationController.cpp

    # resources
    resources/qml.qrc
)

target_include_directories(ChatRoomClient PRIVATE src)
target_link_libraries(ChatRoomClient PRIVATE
    Qt6::Core Qt6::Quick Qt6::Network Qt6::Sql
)
