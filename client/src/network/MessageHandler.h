#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>


namespace client {

class MessageHandler : public QObject
{
    Q_OBJECT

public:
    explicit MessageHandler(QObject *parent = nullptr);
    ~MessageHandler() override;

signals:

public slots:

private:

};

} // namespace client

#endif // MESSAGEHANDLER_H
