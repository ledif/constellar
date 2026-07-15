#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QProcessEnvironment>
#include <QTextStream>

#include "DBusConstants.h"
#include "ManagerAdaptor.h"
#include "ManagerService.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("wowcapd"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("wowcapd - WoW combat log watcher and recording daemon"));
    parser.addHelpOption();
    const QCommandLineOption logDirOption(
        QStringList{QStringLiteral("log-dir")},
        QStringLiteral("Path to the WoW Logs directory to watch (falls back to "
                       "WOWCAPD_LOG_DIR if unset)."),
        QStringLiteral("path"));
    parser.addOption(logDirOption);
    parser.process(app);

    QString logDirectory = parser.value(logDirOption);
    if (logDirectory.isEmpty()) {
        logDirectory =
            QProcessEnvironment::systemEnvironment().value(QStringLiteral("WOWCAPD_LOG_DIR"));
    }
    if (logDirectory.isEmpty()) {
        QTextStream(stderr) << "wowcapd: no log directory given. Pass --log-dir <path> or set "
                               "WOWCAPD_LOG_DIR.\n";
        return 1;
    }

    auto *service = new ManagerService(logDirectory, &app);
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

    if (!service->start()) {
        qCritical() << "Failed to watch log directory" << logDirectory;
        return 1;
    }

    qInfo() << "wowcapd running, State =" << service->state() << "watching" << logDirectory;
    return app.exec();
}
