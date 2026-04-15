#include "AppCore.h"
#include <QtQml/qqmlregistration.h>

namespace client {

AppCore::AppCore(QObject *parent)
    : QObject(parent)
{
}

AppCore::~AppCore()
{
}

} // namespace client
