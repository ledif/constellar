#pragma once

#include <QDBusObjectPath>
#include <QMap>
#include <QMetaType>
#include <QString>
#include <QVariantMap>

// The nested a{sa{sv}} / a{oa{sa{sv}}} envelopes org.freedesktop.DBus.ObjectManager
// needs, which Qt has no built-in metatype for. Names follow the bluez-qt precedent
// TASK-008's XML QtTypeName annotations reference.
using QVariantMapMap = QMap<QString, QVariantMap>;
using DBusManagerStruct = QMap<QDBusObjectPath, QVariantMapMap>;

Q_DECLARE_METATYPE(QVariantMapMap)
Q_DECLARE_METATYPE(DBusManagerStruct)

// Registers the marshallers for the two aliases above. Call once before an
// ObjectManagerAdaptor is registered on the bus.
void registerManagedTypes();
