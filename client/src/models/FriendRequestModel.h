#ifndef FRIENDREQUESTMODEL_H
#define FRIENDREQUESTMODEL_H

#include <QAbstractListModel>

namespace client {

class FriendRequestModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FriendRequestModel(QObject *parent = nullptr);
    ~FriendRequestModel() override;

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    QHash<int, QByteArray> m_roles;
};

} // namespace client

#endif // FRIENDREQUESTMODEL_H
