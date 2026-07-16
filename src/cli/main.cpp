#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusReply>
#include <QMapIterator>
#include <QTextStream>
#include <QVariantMap>
#include <cstdio>

#include "DBusConstants.h"
#include "observerproxy.h"

namespace {

void printUsage() {
    QTextStream(stdout) << "usage: constellar <status>\n";
}

int runStatus() {
    ObserverProxy manager(constellar::dbus::kServiceName, constellar::dbus::kObjectPath,
                          QDBusConnection::sessionBus());
    if (!manager.isValid()) {
        QTextStream(stderr) << "constellar: cannot reach constellard: "
                            << manager.lastError().message() << "\n";
        return 1;
    }

    QDBusReply<QVariantMap> reply = manager.Status();
    if (!reply.isValid()) {
        QTextStream(stderr) << "constellar: Status() failed: " << reply.error().message() << "\n";
        return 1;
    }

    QTextStream out(stdout);
    const QVariantMap status = reply.value();
    QMapIterator<QString, QVariant> it(status);
    while (it.hasNext()) {
        it.next();
        out << it.key() << ": " << it.value().toString() << "\n";
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
    if (command == QStringLiteral("status")) {
        return runStatus();
    }

    printUsage();
    return 1;
}
