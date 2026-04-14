#ifndef MESSAGEDAO_H
#define MESSAGEDAO_H

#include <QObject>


namespace client {

class MessageDAO : public QObject
{
    Q_OBJECT

public:
    explicit MessageDAO(QObject *parent = nullptr);
    ~MessageDAO() override;

signals:

public slots:

private:

};

} // namespace client

#endif // MESSAGEDAO_H
