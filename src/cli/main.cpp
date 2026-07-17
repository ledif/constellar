#include <QCoreApplication>
#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QVariantMap>
#include <cstdio>

#include "ActivityKeys.h"
#include "DBusConstants.h"
#include "observerproxy.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

namespace {

void printUsage() {
    QTextStream(stdout) << "usage: constellar status [--json]\n";
}

QString activitySummary(const QVariantMap &activity) {
    const QString type = activity.value(QString::fromLatin1(keys::kType)).toString();
    if (type == QString::fromLatin1(keys::kTypeEncounter)) {
        return u"encounter %1 %2"_s.arg(
            activity.value(QString::fromLatin1(keys::kDifficulty)).toString(),
            activity.value(QString::fromLatin1(keys::kEncounterName)).toString());
    }
    if (type == QString::fromLatin1(keys::kTypeDungeon)) {
        return u"dungeon +%1"_s.arg(
            activity.value(QString::fromLatin1(keys::kKeystoneLevel)).toString());
    }
    return u"none"_s;
}

QString zoneSummary(const QVariantMap &zone) {
    if (zone.isEmpty()) {
        return u"none"_s;
    }
    return u"%1 (mapId %2)"_s.arg(zone.value(QString::fromLatin1(keys::kZoneName)).toString(),
                                  zone.value(QString::fromLatin1(keys::kMapId)).toString());
}

int runStatus(bool json) {
    ObserverProxy manager(constellar::dbus::kServiceName, constellar::dbus::kObjectPath,
                          QDBusConnection::sessionBus());
    if (!manager.isValid()) {
        QTextStream(stderr) << "constellar: cannot reach constellard: "
                            << manager.lastError().message() << "\n";
        return 1;
    }

    const QVariantMap activity = manager.property("Activity").toMap();
    const QVariantMap zone = manager.property("Zone").toMap();

    QTextStream out(stdout);
    if (json) {
        QJsonObject root;
        root["activity"] = QJsonObject::fromVariantMap(activity);
        root["zone"] = QJsonObject::fromVariantMap(zone);
        out << QJsonDocument(root).toJson(QJsonDocument::Compact) << "\n";
    } else {
        out << "Activity: " << (activity.isEmpty() ? u"none"_s : activitySummary(activity)) << "\n";
        out << "Zone:     " << zoneSummary(zone) << "\n";
    }
    return 0;
}

}  // namespace

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        printUsage();
        return 1;
    }

    const QString command = QString::fromLocal8Bit(argv[1]);
    if (command == u"status"_s) {
        const bool json = argc >= 3 && QString::fromLocal8Bit(argv[2]) == u"--json"_s;
        return runStatus(json);
    }

    printUsage();
    return 1;
}
