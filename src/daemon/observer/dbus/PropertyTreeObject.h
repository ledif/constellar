#pragma once

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVirtualObject>
#include <QString>
#include <QVariantMap>

class InterfaceRegistry;
class ObjectTreePublisher;

// Serves the whole /activity subtree generically: property shape comes from
// InterfaceRegistry (the canonical XML), property values come from
// ObjectTreePublisher::resolve(). Registered once with QDBusConnection::SubPath
// and never (un)registered per-activity -- the resolver returning nullopt is
// what produces "no such object" for an idle activity slot.
class PropertyTreeObject : public QDBusVirtualObject
{
    Q_OBJECT

  public:
    PropertyTreeObject(
        InterfaceRegistry const& registry, ObjectTreePublisher const& publisher,
        QDBusConnection connection, QObject* parent = nullptr
    );

    QString introspect(QString const& path) const override;
    bool handleMessage(QDBusMessage const& message, QDBusConnection const& connection) override;

    // Builds and sends org.freedesktop.DBus.Properties.PropertiesChanged manually
    // on the registration connection. No call sites yet -- TASK-010's tool.
    void emitPropertiesChanged(
        QString const& path, QString const& interfaceName, QVariantMap const& changedProperties
    );

    // The message-construction half of emitPropertiesChanged, split out so its
    // shape (member/interface/args) is unit-testable without a live connection.
    static QDBusMessage buildPropertiesChangedMessage(
        QString const& path, QString const& interfaceName, QVariantMap const& changedProperties
    );

  private:
    bool handleProperties(QDBusMessage const& message, QDBusConnection const& connection);
    bool handleIntrospectable(QDBusMessage const& message, QDBusConnection const& connection);

    QString introspectionXmlFor(QString const& subPath) const;

    InterfaceRegistry const& m_registry;
    ObjectTreePublisher const& m_publisher;
    QDBusConnection m_connection;
};
