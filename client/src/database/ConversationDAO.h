#ifndef CONVERSATIONDAO_H
#define CONVERSATIONDAO_H

#include <QObject>


namespace client {

class ConversationDAO : public QObject
{
    Q_OBJECT

public:
    explicit ConversationDAO(QObject *parent = nullptr);
    ~ConversationDAO() override;

signals:

public slots:

private:

};

} // namespace client

#endif // CONVERSATIONDAO_H
