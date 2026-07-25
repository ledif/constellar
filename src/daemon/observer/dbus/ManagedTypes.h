#pragma once

#include <QDBusObjectPath>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <QVariantMap>

// Nested a{sa{sv}} and a{oa{sa{sv}}} types that org.freedesktop.DBus.ObjectManager uses
// (stolen from bluez-qt)
using QVariantMapMap = QMap<QString, QVariantMap>;
using DBusManagerStruct = QMap<QDBusObjectPath, QVariantMapMap>;

Q_DECLARE_METATYPE(QVariantMapMap)
Q_DECLARE_METATYPE(DBusManagerStruct)

// Registers the marshallers for the two aliases above.
void registerManagedTypes();
