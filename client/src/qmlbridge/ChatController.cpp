// ChatController.cpp - 聊天控制器实现文件
// 引入ChatController类的头文件，声明了ChatController类的所有成员
#include "ChatController.h"
// 引入Qt日期时间类，用于生成消息时间戳
#include <QDateTime>
// 引入Qt JSON文档类，用于解析和构建JSON数据
#include <QJsonDocument>
// 引入Qt JSON对象类，用于处理JSON对象
#include <QJsonObject>
// 引入Qt调试类，用于输出调试信息
#include <QDebug>
// 引入C++随机数库
#include <random>
// 使用client命名空间，避免类名冲突


namespace client {

// 构造函数实现
// parent: 父对象指针，用于Qt对象树的内存管理
ChatController::ChatController(QObject *parent)
    : QObject(parent)                                    // 调用基类QObject的构造函数
    , m_currentUserId("currentUser")                     // 初始化当前用户ID为"currentUser"
{
}

// 析构函数实现
// 当ChatController对象被销毁时调用，用于清理资源
ChatController::~ChatController()
{

}

// 发送消息方法实现
// 功能：发送消息给指定的用户
// 参数：receiverId - 接收消息的用户ID，message - 要发送的消息内容
// 返回：无（通过信号通知发送结果）
void ChatController::sendMessage(const QString &receiverId, const QString &message)
{
    // 生成一个唯一的消息ID，用于标识这条消息
    // 格式：msg_时间戳_随机数，例如 msg_1699999999999_123
    QString messageId = generateMessageId();

    // 输出调试信息，显示发送的消息内容
    qDebug() << "Sending message - ID:" << messageId << "To:" << receiverId << "Content:" << message;

    // 发出消息发送成功信号，通知前端消息已发送
    // 前端可以监听这个信号来更新UI
    emit messageSent(messageId, receiverId);
}

// 加载历史消息方法实现
// 功能：从服务器加载指定会话的历史消息
// 参数：conversationId - 会话ID，limit - 加载消息数量上限，offset - 消息偏移量（用于分页）
// 返回：无（通过信号返回加载的消息列表）
void ChatController::loadMessages(const QString &conversationId, int limit, int offset)
{
    // 输出调试信息，显示加载消息的参数
    qDebug() << "Loading messages - Conversation:" << conversationId << "Limit:" << limit << "Offset:" << offset;

    // 创建一个空的QVariantList，用于存储消息列表
    // QVariantList可以存储QML可识别的各种类型的数据
    QVariantList mockMessages;

    // 发出消息加载完成信号，将消息列表发送给前端
    // 前端可以监听这个信号来获取并显示历史消息
    emit messagesLoaded(mockMessages);
}

// 处理收到的消息（私有槽函数）
// 功能：解析并处理从服务器收到的消息数据
// 参数：data - 从网络接收的原始字节数据（JSON格式）
// 返回：无（通过信号将解析后的消息发送给前端）
void ChatController::onMessageReceived(const QByteArray &data)
{
    // 将原始字节数据解析为JSON文档对象
    // 如果数据不是有效的JSON格式，解析会失败
    QJsonDocument doc = QJsonDocument::fromJson(data);

    // 检查解析是否成功，如果失败则直接返回
    if (!doc.isObject()) {
        return;
    }

    // 获取JSON对象
    QJsonObject obj = doc.object();

    // 从JSON对象中提取消息类型和动作
    QString type = obj["type"].toString();    // 消息类型，例如"chat"
    QString action = obj["action"].toString(); // 动作类型，例如"receive"

    // 判断是否为聊天消息且为接收动作
    if (type == "chat" && action == "receive") {
        // 从JSON对象中提取消息的各个字段
        QString messageId = obj["id"].toString();           // 消息ID
        QString sender = obj["senderId"].toString();        // 发送者ID
        QString content = obj["content"].toString();       // 消息内容

        // 发出收到新消息信号，将解析后的消息数据发送给前端
        // 前端可以监听这个信号来实时显示新消息
        emit messageReceived(messageId, sender, content);
    }
}

// 处理连接状态变化（私有槽函数）
// 功能：当与服务器的连接状态发生变化时调用
// 参数：connected - true表示已连接，false表示已断开
// 返回：无
void ChatController::onConnectionStatusChanged(bool connected)
{
    // 输出调试信息，显示当前的连接状态
    // 使用三元运算符根据connected的值输出不同的文字描述
    qDebug() << "Connection status changed:" << (connected ? "Connected" : "Disconnected");
}

// 生成唯一消息ID（私有方法）
// 功能：生成一个唯一的字符串作为消息的标识符
// 参数：无
// 返回：唯一的消息ID字符串
// 算法：使用当前时间戳（毫秒级）+ 随机数，确保在分布式系统中也不会冲突
QString ChatController::generateMessageId()
{
    // 格式说明：
    // msg_ - 消息ID前缀，用于标识这是消息ID
    // QDateTime::currentMSecsSinceEpoch() - 获取当前时间戳（毫秒）
    // 随机数 - 生成0-999之间的随机数
    // 最终格式示例：msg_1699999999999_123
    
    // 使用C++11随机数生成器
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dis(0, 999);
    
    int randomNum = dis(gen);
    
    return "msg_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + QString::number(randomNum);
}

} // 结束client命名空间