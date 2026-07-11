#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#include "DBusConstants.h"
#include "ManagerAdaptor.h"
#include "ManagerService.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("wowcapd"));

    auto *service = new ManagerService(&app);
    new ManagerAdaptor(service);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(wowcapd::dbus::kObjectPath, service)) {
        qCritical() << "Failed to register DBus object at" << wowcapd::dbus::kObjectPath << ":"
                    << bus.lastError().message();
        return 1;
    }
    if (!bus.registerService(wowcapd::dbus::kServiceName)) {
        qCritical() << "Failed to acquire DBus service name" << wowcapd::dbus::kServiceName << ":"
                    << bus.lastError().message();
        return 1;
    }

    qInfo() << "wowcapd running, State =" << service->state();
    return app.exec();
}
