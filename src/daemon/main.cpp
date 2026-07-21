#include <cstdio>
#include <filesystem>

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
#include "ObserverDBusAdaptor.h"
#include "ObserverService.h"
#include "PresencePublisher.h"

using namespace Qt::StringLiterals;

namespace
{

constexpr QStringView kDefaultDiscordAppId = u"1527462779290652672";

}  // namespace

int main(int argc, char* argv[])
{
    // if we're not running with journald, force stderr to be flushed
    if (!qEnvironmentVariableIsSet("JOURNAL_STREAM"))
        qputenv("QT_FORCE_STDERR_LOGGING", "1");

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(u"constellard"_s);

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Word of Warcraft combat log watcher and recording daemon"_s);

    parser.addHelpOption();
    QCommandLineOption const logDirOption(
        QStringList{u"log-dir"_s},
        u"Path to the WoW Logs directory (override: CONSTELLAR_LOG_DIR)"_s, u"path"_s
    );

    parser.addOption(logDirOption);
    parser.process(app);

    QString logDirectory = parser.value(logDirOption);
    if (logDirectory.isEmpty())
        logDirectory = QProcessEnvironment::systemEnvironment().value(u"CONSTELLAR_LOG_DIR"_s);

    if (logDirectory.isEmpty())
    {
        QTextStream(stderr) << "constellard: no log directory given\n";
        return 1;
    }

    auto* service =
        new ObserverService(std::filesystem::path(logDirectory.toStdString()), /*config*/ {}, &app);
    new ObserverDBusAdaptor(service);

    QString discordAppId =
        QProcessEnvironment::systemEnvironment().value(u"CONSTELLAR_DISCORD_APP_ID"_s);

    if (discordAppId.isEmpty())
        discordAppId = kDefaultDiscordAppId.toString();

    auto* discordClient = new DiscordIpcClient(discordAppId, &app);
    auto* presence = new PresencePublisher(*discordClient, service->gameState(), &app);

    // send game state changes to Discord
    QObject::connect(
        &service->gameState(), &GameState::activityChanged, presence,
        &PresencePublisher::onActivityChanged
    );

    QObject::connect(
        &service->gameState(), &GameState::locationChanged, presence,
        &PresencePublisher::onLocationChanged
    );

    QObject::connect(
        &app, &QCoreApplication::aboutToQuit, presence,
        [discordClient]() { discordClient->clearActivity(); }
    );

    discordClient->start();

    // register our daemon with D-Bus
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
