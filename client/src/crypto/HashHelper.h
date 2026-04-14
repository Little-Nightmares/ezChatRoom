#ifndef HASHHELPER_H
#define HASHHELPER_H

#include <QObject>


namespace client {

class HashHelper : public QObject
{
    Q_OBJECT

public:
    explicit HashHelper(QObject *parent = nullptr);
    ~HashHelper() override;

signals:

public slots:

private:

};

} // namespace client

#endif // HASHHELPER_H
