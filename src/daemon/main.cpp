#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QProcessEnvironment>
#include <QTextStream>

#include "DBusConstants.h"
#include "DiscordIpcClient.h"
#include "ManagerAdaptor.h"
#include "ManagerService.h"
#include "PresencePublisher.h"

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
    const QCommandLineOption discordAppIdOption(
        QStringList{QStringLiteral("discord-app-id")},
        QStringLiteral("Discord Application ID to publish Rich Presence as (falls back to "
                       "WOWCAPD_DISCORD_APP_ID if unset). Presence is off unless this is set "
                       "(RFC-002) -- no app has been registered yet."),
        QStringLiteral("id"));
    parser.addOption(discordAppIdOption);
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

    QString discordAppId = parser.value(discordAppIdOption);
    if (discordAppId.isEmpty()) {
        discordAppId = QProcessEnvironment::systemEnvironment().value(
            QStringLiteral("WOWCAPD_DISCORD_APP_ID"));
    }
    // Off by default, config-gated (RFC-002): the publisher and IPC client
    // no-op harmlessly with an empty app ID, but skip constructing them
    // entirely when presence isn't configured.
    if (!discordAppId.isEmpty()) {
        auto *discordClient = new DiscordIpcClient(discordAppId, &app);
        auto *presence = new PresencePublisher(*discordClient, &app);
        QObject::connect(service, &ManagerService::encounterDetected, presence,
                         &PresencePublisher::onEncounterDetected);
        QObject::connect(service, &ManagerService::encounterEnded, presence,
                         &PresencePublisher::onEncounterEnded);
        QObject::connect(service, &ManagerService::dungeonDetected, presence,
                         &PresencePublisher::onDungeonDetected);
        QObject::connect(service, &ManagerService::dungeonEnded, presence,
                         &PresencePublisher::onDungeonEnded);
        QObject::connect(service, &ManagerService::stateChanged, presence,
                         &PresencePublisher::onStateChanged);
        discordClient->start();
    }

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
