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
#include "GameState.h"
#include "ObserverAdaptor.h"
#include "ObserverService.h"
#include "PresencePublisher.h"

using namespace Qt::StringLiterals;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(u"constellard"_s);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        u"constellard - WoW combat log watcher and recording daemon"_s
    );
    parser.addHelpOption();
    QCommandLineOption const logDirOption(
        QStringList{u"log-dir"_s},
        u"Path to the WoW Logs directory to watch (falls back to "
        "CONSTELLAR_LOG_DIR if unset)."_s,
        u"path"_s
    );
    parser.addOption(logDirOption);
    QCommandLineOption const discordAppIdOption(
        QStringList{u"discord-app-id"_s},
        u"Discord Application ID to publish Rich Presence as (falls back to "
        "CONSTELLAR_DISCORD_APP_ID if unset). Presence is off unless this is set "
        "(RFC-002) -- no app has been registered yet."_s,
        u"id"_s
    );
    parser.addOption(discordAppIdOption);
    parser.process(app);

    QString logDirectory = parser.value(logDirOption);
    if (logDirectory.isEmpty())
        logDirectory = QProcessEnvironment::systemEnvironment().value(u"CONSTELLAR_LOG_DIR"_s);
    if (logDirectory.isEmpty())
    {
        QTextStream(stderr) << "constellard: no log directory given. Pass --log-dir <path> or set "
                               "CONSTELLAR_LOG_DIR.\n";
        return 1;
    }

    auto* service = new ObserverService(logDirectory, &app);
    new ObserverAdaptor(service);

    QString discordAppId = parser.value(discordAppIdOption);
    if (discordAppId.isEmpty())
    {
        discordAppId =
            QProcessEnvironment::systemEnvironment().value(u"CONSTELLAR_DISCORD_APP_ID"_s);
    }
    // Off by default, config-gated (RFC-002): the publisher and IPC client
    // no-op harmlessly with an empty app ID, but skip constructing them
    // entirely when presence isn't configured.
    if (!discordAppId.isEmpty())
    {
        auto* discordClient = new DiscordIpcClient(discordAppId, &app);
        auto* presence = new PresencePublisher(*discordClient, service->gameState(), &app);
        QObject::connect(
            &service->gameState(), &GameState::activityChanged, presence,
            &PresencePublisher::onActivityChanged
        );
        QObject::connect(
            &service->gameState(), &GameState::zoneChanged, presence,
            &PresencePublisher::onZoneChanged
        );
        QObject::connect(
            &app, &QCoreApplication::aboutToQuit, presence,
            [discordClient]() { discordClient->clearActivity(); }
        );
        discordClient->start();
    }

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerObject(constellar::dbus::kObjectPath, service))
    {
        qCritical() << "Failed to register DBus object at" << constellar::dbus::kObjectPath << ":"
                    << bus.lastError().message();
        return 1;
    }
    if (!bus.registerService(constellar::dbus::kServiceName))
    {
        qCritical() << "Failed to acquire DBus service name" << constellar::dbus::kServiceName
                    << ":" << bus.lastError().message();
        return 1;
    }

    if (!service->start())
    {
        qCritical() << "Failed to watch log directory" << logDirectory;
        return 1;
    }

    qInfo() << "constellard running, watching" << logDirectory;
    return app.exec();
}
