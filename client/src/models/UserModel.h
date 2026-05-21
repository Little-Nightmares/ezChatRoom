#ifndef USERMODEL_H
#define USERMODEL_H

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

namespace client {

class UserModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit UserModel(QObject *parent = nullptr);
    ~UserModel() override;

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void setUsers(const QVariantList &users);
    void appendUser(const QVariantMap &user);
    void clear();

protected:
    QHash<int, QByteArray> m_roles;
    QVector<QVariantMap> m_users;
};

} // namespace client

#endif // USERMODEL_H
