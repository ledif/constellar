#include "PropertyTreeObjectTest.h"

#include <QDBusMessage>
#include <QTest>

#include "DBusConstants.h"
#include "dbus/PropertyTreeObject.h"

namespace dbus = constellar::dbus;

void PropertyTreeObjectTest::propertiesChangedMessageHasExpectedShape()
{
    QVariantMap const changed{{QStringLiteral("Type"), QStringLiteral("encounter")}};

    QDBusMessage const signal = PropertyTreeObject::buildPropertiesChangedMessage(
        dbus::kActivityObjectPath, dbus::kActivityInterfaceName, changed
    );

    QCOMPARE(signal.type(), QDBusMessage::SignalMessage);
    QCOMPARE(signal.path(), dbus::kActivityObjectPath);
    QCOMPARE(signal.interface(), QStringLiteral("org.freedesktop.DBus.Properties"));
    QCOMPARE(signal.member(), QStringLiteral("PropertiesChanged"));

    QList<QVariant> const args = signal.arguments();
    QCOMPARE(args.size(), 3);
    QCOMPARE(args.at(0).toString(), dbus::kActivityInterfaceName);
    QCOMPARE(args.at(1).toMap(), changed);
    QVERIFY(args.at(2).toStringList().isEmpty());
}

QTEST_MAIN(PropertyTreeObjectTest)
