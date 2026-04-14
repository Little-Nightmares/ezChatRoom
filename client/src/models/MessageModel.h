#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>

namespace client {

class MessageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit MessageModel(QObject *parent = nullptr);
    ~MessageModel() override;

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    QHash<int, QByteArray> m_roles;
};

} // namespace client

#endif // MESSAGEMODEL_H
