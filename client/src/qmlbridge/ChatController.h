#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QObject>


namespace client {

class ChatController : public QObject
{
    Q_OBJECT

public:
    explicit ChatController(QObject *parent = nullptr);
    ~ChatController() override;

signals:

public slots:

private:

};

} // namespace client

#endif // CHATCONTROLLER_H
