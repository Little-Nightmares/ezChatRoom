#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include <QAbstractListModel>

namespace client {

class ConversationModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ConversationModel(QObject *parent = nullptr);
    ~ConversationModel() override;

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    QHash<int, QByteArray> m_roles;
};

} // namespace client

#endif // CONVERSATIONMODEL_H
