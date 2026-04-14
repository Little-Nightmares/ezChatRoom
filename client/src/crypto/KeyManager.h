#ifndef KEYMANAGER_H
#define KEYMANAGER_H

#include <QObject>


namespace client {

class KeyManager : public QObject
{
    Q_OBJECT

public:
    explicit KeyManager(QObject *parent = nullptr);
    ~KeyManager() override;

signals:

public slots:

private:

};

} // namespace client

#endif // KEYMANAGER_H
