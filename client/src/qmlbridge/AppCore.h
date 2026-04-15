#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>


namespace client {

class AppCore : public QObject
{
    Q_OBJECT

public:
    explicit AppCore(QObject *parent = nullptr);
    ~AppCore() override;

signals:

public slots:

private:

};

} // namespace client

#endif // APPCORE_H
