#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include "core/TcpServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("ChatRoomServer"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("ChatRoom LAN messaging server"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption portOption(QStringList() << QStringLiteral("p") << QStringLiteral("port"),
                                  QStringLiteral("TCP listen port."),
                                  QStringLiteral("port"),
                                  QStringLiteral("6667"));
    parser.addOption(portOption);
    parser.process(app);

    bool ok = false;
    const int requestedPort = parser.value(portOption).toInt(&ok);
    if (!ok || requestedPort < 1 || requestedPort > 65535) {
        qCritical() << "Invalid port:" << parser.value(portOption);
        return 1;
    }

    server::TcpServer tcpServer;
    if (!tcpServer.start(static_cast<quint16>(requestedPort)))
        return 1;

    QObject::connect(&app, &QCoreApplication::aboutToQuit,
                     &tcpServer, &server::TcpServer::stop);

    return app.exec();
}
