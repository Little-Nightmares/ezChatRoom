#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

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

public slots:
    void setMessages(const QVariantList &messages);
    void appendMessage(const QVariantMap &message);
    void clear();

protected:
    QHash<int, QByteArray> m_roles;
    QVector<QVariantMap> m_messages;
};

} // namespace client

#endif // MESSAGEMODEL_H
