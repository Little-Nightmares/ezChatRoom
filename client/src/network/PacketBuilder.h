#ifndef PACKETBUILDER_H
#define PACKETBUILDER_H

#include <QObject>


namespace client {

class PacketBuilder : public QObject
{
    Q_OBJECT

public:
    explicit PacketBuilder(QObject *parent = nullptr);
    ~PacketBuilder() override;

signals:

public slots:

private:

};

} // namespace client

#endif // PACKETBUILDER_H
