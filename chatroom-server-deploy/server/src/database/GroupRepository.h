#ifndef GROUPREPOSITORY_H
#define GROUPREPOSITORY_H

#include <QObject>


namespace server {

class GroupRepository : public QObject
{
    Q_OBJECT

public:
    explicit GroupRepository(QObject *parent = nullptr);
    ~GroupRepository() override;

signals:

public slots:

private:
};

} // namespace server

#endif // GROUPREPOSITORY_H
