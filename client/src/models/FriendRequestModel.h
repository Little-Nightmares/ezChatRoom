#ifndef FRIENDREQUESTMODEL_H
#define FRIENDREQUESTMODEL_H

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

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

public slots:
    void setRequests(const QVariantList &requests);
    void appendRequest(const QVariantMap &request);
    void clear();

protected:
    QHash<int, QByteArray> m_roles;
    QVector<QVariantMap> m_requests;
};

} // namespace client

#endif // FRIENDREQUESTMODEL_H
