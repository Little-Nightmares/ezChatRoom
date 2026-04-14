#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>


namespace client {

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject *parent = nullptr);
    ~SettingsManager() override;

signals:

public slots:

private:

};

} // namespace client

#endif // SETTINGSMANAGER_H
