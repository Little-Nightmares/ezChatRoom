set(CLIENT_SOURCES
    main.cpp
    src/core/AppManager.h
    src/core/AppManager.cpp
    src/core/SessionManager.h
    src/core/SessionManager.cpp
    src/network/TcpClient.h
    src/network/TcpClient.cpp
    src/network/MessageHandler.h
    src/network/MessageHandler.cpp
    src/network/HeartbeatManager.h
    src/network/HeartbeatManager.cpp
    src/database/DatabaseManager.h
    src/database/DatabaseManager.cpp
    src/models/MessageModel.h
    src/models/MessageModel.cpp
    src/models/UserModel.h
    src/models/UserModel.cpp
    src/models/ConversationModel.h
    src/models/ConversationModel.cpp
    src/models/FriendRequestModel.h
    src/models/FriendRequestModel.cpp
    src/qmlbridge/AppCore.h
    src/qmlbridge/AppCore.cpp
    src/qmlbridge/ChatController.h
    src/qmlbridge/ChatController.cpp
    src/qmlbridge/UserController.h
    src/qmlbridge/UserController.cpp
    src/qmlbridge/FriendController.h
    src/qmlbridge/FriendController.cpp
    src/qmlbridge/GroupController.h
    src/qmlbridge/GroupController.cpp
    src/qmlbridge/ThemeManager.h
    src/qmlbridge/ThemeManager.cpp
    src/models/GroupModel.h
    src/models/GroupModel.cpp
    resources/sounds/sounds.qrc
    resources/fonts/fonts.qrc
)

find_package(Qt6 REQUIRED COMPONENTS Core Network Sql Quick Gui QuickControls2)
find_package(Qt6 OPTIONAL_COMPONENTS Multimedia)

qt_add_executable(ChatRoomClient ${CLIENT_SOURCES})

qt_add_qml_module(ChatRoomClient
    URI ChatRoom
    VERSION 1.0
    RESOURCE_PREFIX /
    QML_FILES
        qml/main.qml
        qml/pages/LoginPage.qml
        qml/pages/RegisterPage.qml
        qml/pages/ChatPage.qml
        qml/pages/FriendRequestPage.qml
        qml/pages/SettingsPage.qml
        qml/components/MessageBubble.qml
        qml/components/ContactItem.qml
        qml/components/ConversationItem.qml
        qml/components/InputBar.qml
        qml/components/Avatar.qml
        qml/components/SearchBar.qml
        qml/components/Toast.qml
        qml/dialogs/AddFriendDialog.qml
        qml/dialogs/ConfirmDialog.qml
        qml/dialogs/CreateGroupDialog.qml
        qml/dialogs/GroupInfoDialog.qml
        qml/components/GroupMessageBubble.qml
        qml/components/GroupConversationItem.qml
        qml/components/StyledText.qml
        qml/components/EmojiPicker.qml
)

target_include_directories(ChatRoomClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database
    ${CMAKE_CURRENT_SOURCE_DIR}/src/models
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qmlbridge
    ${CMAKE_SOURCE_DIR}/common/src
)

target_link_libraries(ChatRoomClient PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::Sql
    CommonLib
)

if(Qt6Multimedia_FOUND)
    target_link_libraries(ChatRoomClient PRIVATE Qt6::Multimedia)
    target_compile_definitions(ChatRoomClient PRIVATE CHATROOM_HAS_MULTIMEDIA)
endif()

# Windows system libraries are linked via CommonLib -> OpenSSL::Crypto INTERFACE
