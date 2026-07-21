#include <QCoreApplication>
#include <QDBusConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QVariantMap>
#include <cstdio>

#include "Activity.h"
#include "ActivityKeys.h"
#include "DBusConstants.h"
#include "Location.h"
#include "observerproxy.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace
{

void printUsage()
{
    QTextStream(stdout) << "usage: constellarctl status [--json]\n";
}

// QJsonObject::fromVariantMap can't render uiMapBounds: in-process it's a QList<double>,
// and after a D-Bus round trip it's an undemarshalled QDBusArgument — neither converts
// to QJsonValue automatically, so build the JSON object from the decoded Location instead.
QJsonObject locationToJson(Location const& location)
{
    QJsonObject json;

    if (std::optional<UiMap> const& uiMap = location.uiMap())
    {
        json[QString::fromLatin1(keys::kUiMapId)] = static_cast<qint64>(uiMap->id);
        json[QString::fromLatin1(keys::kUiMapName)] = uiMap->name;
        if (uiMap->bounds.isValid())
        {
            json[QString::fromLatin1(keys::kUiMapBounds)] =
                QJsonArray{uiMap->bounds.x0, uiMap->bounds.x1, uiMap->bounds.y0, uiMap->bounds.y1};
        }
    }

    if (std::optional<Zone> const& zone = location.zone())
    {
        json[QString::fromLatin1(keys::kZoneInstanceId)] = static_cast<qint64>(zone->instanceId);
        json[QString::fromLatin1(keys::kZoneName)] = zone->name;
        json[QString::fromLatin1(keys::kZoneDifficultyId)] =
            static_cast<qint64>(zone->difficultyId);
    }

    return json;
}

int runStatus(bool json)
{
    ObserverProxy manager(
        constellar::dbus::kServiceName, constellar::dbus::kObjectPath, QDBusConnection::sessionBus()
    );

    if (!manager.isValid())
    {
        QTextStream(stderr) << "constellarctl: cannot reach constellard: "
                            << manager.lastError().message() << "\n";
        return 1;
    }

    QVariantMap const activityMap = manager.property("Activity").toMap();
    Location const location = Location::fromVariantMap(manager.property("Location").toMap());

    QTextStream out(stdout);
    if (json)
    {
        QJsonObject root;
        root["activity"] = QJsonObject::fromVariantMap(activityMap);
        root["location"] = locationToJson(location);
        out << QJsonDocument(root).toJson(QJsonDocument::Compact) << "\n";
    }
    else
    {
        out << "Activity: " << Activity::fromVariantMap(activityMap).toString() << "\n";
        out << "Location: " << location.toString() << "\n";
    }

    return 0;
}

}  // namespace

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    if (argc < 2)
    {
        printUsage();
        return 1;
    }

    QString const command = QString::fromLocal8Bit(argv[1]);
    if (command == u"status"_s)
    {
        bool const json = argc >= 3 && QString::fromLocal8Bit(argv[2]) == u"--json"_s;
        return runStatus(json);
    }

    printUsage();
    return 1;
}
