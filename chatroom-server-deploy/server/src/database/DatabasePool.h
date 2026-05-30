#ifndef DATABASEPOOL_H
#define DATABASEPOOL_H

#include <QObject>


namespace server {

class DatabasePool : public QObject
{
    Q_OBJECT

public:
    explicit DatabasePool(QObject *parent = nullptr);
    ~DatabasePool() override;

signals:

public slots:

private:
};

} // namespace server

#endif // DATABASEPOOL_H
