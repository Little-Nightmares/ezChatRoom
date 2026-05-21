#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

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

public slots:
    void setConversations(const QVariantList &conversations);
    void appendConversation(const QVariantMap &conversation);
    void clear();

protected:
    QHash<int, QByteArray> m_roles;
    QVector<QVariantMap> m_conversations;
};

} // namespace client

#endif // CONVERSATIONMODEL_H
