#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QDebug>
#include "qmlbridge/AppCore.h"

using chatroom::client::AppCore;

int main(int argc, char* argv[])
{
    // Use Fusion style so Button background/contentItem customizations work
    QQuickStyle::setStyle("Fusion");

    QGuiApplication app(argc, argv);

    app.setApplicationName("ChatRoom");
    app.setOrganizationName("ChatRoom");
    app.setApplicationVersion("1.0.0");

    // Register fonts from Qt resources so they're available throughout the app
    QFontDatabase::addApplicationFont(":/fonts/ZCOOLQingKeHuangYou-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Caveat-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Caveat-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSansSC-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/NotoSansSC-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Inter-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/MPLUSRounded1c-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/MPLUSRounded1c-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Nunito-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Nunito-Bold.ttf");

    // Create the AppCore singleton
    AppCore* appCore = new AppCore(&app);

    // Initialize all subsystems (database, network, message handlers)
    appCore->initialize();

    // Set up QML engine
    QQmlApplicationEngine engine;

    // Expose AppCore to QML via root context
    engine.rootContext()->setContextProperty("appCore", appCore);

    // Add resource path so Qt can find ChatRoom module's qmldir
    engine.addImportPath("qrc:/");

    // Load the main QML file
    const QUrl url(QStringLiteral("qrc:/ChatRoom/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
            qCritical() << "Failed to load QML file:" << url;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
