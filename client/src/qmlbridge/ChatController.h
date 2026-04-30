#ifndef CHATCONTROLLER_H
#define CHATCONTROLLER_H

#include <QObject>// 引入QObject类，用于创建可连接的信号和槽
#include <QString>// 引入QString类，用于处理字符串
#include <QByteArray>// 引入QByteArray类，用于处理字节数组
#include <QVariantList>// 引入QVariantList类，用于存储和传递多个数据类型的列表
#include <QtQml/qqmlregistration.h>

namespace client {

class ChatController : public QObject// 定义ChatController类，继承自QObject类
{
    Q_OBJECT// 宏义QObject类的元对象，用于在QML中使用

public:
    explicit ChatController(QObject *parent = nullptr);// 构造函数，用于初始化ChatController对象
    ~ChatController() override;// 析构函数，用于清理资源，确保在对象被销毁时释放所有占用的资源

    Q_INVOKABLE void sendMessage(const QString &receiverId, const QString &message);// 发送消息方法
    Q_INVOKABLE void loadMessages(const QString &conversationId, int limit, int offset);// 加载历史消息方法

signals:
    void messageSent(const QString &messageId, const QString &receiverId);// 消息发送成功信号
    void messageFailed(const QString &errorMessage);// 消息发送失败信号
    void messageReceived(const QString &messageId, const QString &sender, const QString &content);// 收到新消息信号
    void messagesLoaded(const QVariantList &messages);// 加载历史消息信号

private slots:
    void onMessageReceived(const QByteArray &data);
    void onConnectionStatusChanged(bool connected);// 连接状态变化槽函数

private:
    QString generateMessageId();// 生成唯一消息ID方法
    // 生成一个唯一的字符串作为消息的标识符
    // 算法：使用当前时间戳（毫秒级）+ 随机数，确保在分布式系统中也不会冲突
    QString m_currentUserId;// 当前用户ID，用于生成唯一的消息ID
};

} // namespace client

#endif // CHATCONTROLLER_H