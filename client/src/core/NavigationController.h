#ifndef NAVIGATIONCONTROLLER_H
#define NAVIGATIONCONTROLLER_H

#include <QObject>


namespace client {

class NavigationController : public QObject
{
    Q_OBJECT

public:
    explicit NavigationController(QObject *parent = nullptr);
    ~NavigationController() override;

signals:

public slots:

private:

};

} // namespace client

#endif // NAVIGATIONCONTROLLER_H
