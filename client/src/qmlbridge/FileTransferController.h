#ifndef FILETRANSFERCONTROLLER_H
#define FILETRANSFERCONTROLLER_H

#include <QObject>
#include <QtQml/qqmlregistration.h>


namespace client {

class FileTransferController : public QObject
{
    Q_OBJECT

public:
    explicit FileTransferController(QObject *parent = nullptr);
    ~FileTransferController() override;

signals:

public slots:

private:

};

} // namespace client

#endif // FILETRANSFERCONTROLLER_H
