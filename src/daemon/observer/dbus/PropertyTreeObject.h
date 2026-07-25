#pragma once

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVirtualObject>
#include <QString>
#include <QVariantMap>

class InterfaceRegistry;
class ObjectTreePublisher;

// Serves the /activity subtree
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

    // org.freedesktop.DBus.Properties.PropertiesChanged
    void emitPropertiesChanged(
        QString const& path, QString const& interfaceName, QVariantMap const& changedProperties
    );

    // TODO: public for unit tests, can we refactor?
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
