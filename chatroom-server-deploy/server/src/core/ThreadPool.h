#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>


namespace server {

class ThreadPool : public QObject
{
    Q_OBJECT

public:
    explicit ThreadPool(QObject *parent = nullptr);
    ~ThreadPool() override;

signals:

public slots:

private:
};

} // namespace server

#endif // THREADPOOL_H
