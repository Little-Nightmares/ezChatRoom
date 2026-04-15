#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// Bridge 层
#include "qmlbridge/AppCore.h"
#include "qmlbridge/ChatController.h"
#include "qmlbridge/UserController.h"
#include "qmlbridge/FriendController.h"
#include "qmlbridge/FileTransferController.h"
#include "qmlbridge/NotificationController.h"

// Core 层
#include "core/AppManager.h"

using namespace client;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    // 创建 C++ 对象并注册到 QML
    // 这样在 QML 里可以直接用 UserController.xxx 调用
    auto *appManager = new AppManager(&app);
    auto *appCore = new AppCore(&app);
    auto *chatController = new ChatController(&app);
    auto *userController = new UserController(&app);
    auto *friendController = new FriendController(&app);
    auto *fileTransferController = new FileTransferController(&app);
    auto *notificationController = new NotificationController(&app);

    engine.rootContext()->setContextProperty("AppManager", appManager);
    engine.rootContext()->setContextProperty("AppCore", appCore);
    engine.rootContext()->setContextProperty("ChatController", chatController);
    engine.rootContext()->setContextProperty("UserController", userController);
    engine.rootContext()->setContextProperty("FriendController", friendController);
    engine.rootContext()->setContextProperty("FileTransferController", fileTransferController);
    engine.rootContext()->setContextProperty("NotificationController", notificationController);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
