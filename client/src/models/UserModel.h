#ifndef USERMODEL_H
#define USERMODEL_H

#include <QAbstractListModel>

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

protected:
    QHash<int, QByteArray> m_roles;
};

} // namespace client

#endif // USERMODEL_H
