#include <QCoreApplication>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QVariantMap>
#include <cstdio>

#include "Activity.h"
#include "DBusConstants.h"
#include "Zone.h"
#include "observerproxy.h"

using namespace Qt::StringLiterals;

namespace
{

void printUsage()
{
    QTextStream(stdout) << "usage: constellarctl status [--json]\n";
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
    QVariantMap const zoneMap = manager.property("Zone").toMap();

    QTextStream out(stdout);
    if (json)
    {
        QJsonObject root;
        root["activity"] = QJsonObject::fromVariantMap(activityMap);
        root["zone"] = QJsonObject::fromVariantMap(zoneMap);
        out << QJsonDocument(root).toJson(QJsonDocument::Compact) << "\n";
    }
    else
    {
        out << "Activity: " << Activity::fromVariantMap(activityMap).toString() << "\n";
        out << "Zone:     " << Zone::fromVariantMap(zoneMap).toString() << "\n";
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
