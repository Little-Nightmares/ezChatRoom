#ifndef MESSAGEREPOSITORY_H
#define MESSAGEREPOSITORY_H

#include <QObject>


namespace server {

class MessageRepository : public QObject
{
    Q_OBJECT

public:
    explicit MessageRepository(QObject *parent = nullptr);
    ~MessageRepository() override;

signals:

public slots:

private:
};

} // namespace server

#endif // MESSAGEREPOSITORY_H
