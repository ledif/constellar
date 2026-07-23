#include "PropertyTreeObject.h"

#include <utility>

#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDBusVariant>
#include <QDebug>
#include <QStringList>

#include "DBusConstants.h"
#include "InterfaceRegistry.h"
#include "ObjectTreePublisher.h"

using namespace Qt::StringLiterals;

namespace dbus = constellar::dbus;

namespace
{

QString const kPropertiesInterface = u"org.freedesktop.DBus.Properties"_s;
QString const kIntrospectableInterface = u"org.freedesktop.DBus.Introspectable"_s;

QString const kUnknownObjectError = u"org.freedesktop.DBus.Error.UnknownObject"_s;
QString const kUnknownInterfaceError = u"org.freedesktop.DBus.Error.UnknownInterface"_s;
QString const kUnknownPropertyError = u"org.freedesktop.DBus.Error.UnknownProperty"_s;
QString const kPropertyReadOnlyError = u"org.freedesktop.DBus.Error.PropertyReadOnly"_s;

QVariant coerceValue(QString const& signature, QVariant value)
{
    if (signature == u"x"_s && value.metaType() != QMetaType::fromType<qint64>())
    {
        qWarning() << "PropertyTreeObject: expected qint64 for signature 'x', got"
                   << value.typeName();
        return value.toLongLong();
    }
    if (signature == u"u"_s && value.metaType() != QMetaType::fromType<uint>())
    {
        qWarning() << "PropertyTreeObject: expected uint for signature 'u', got"
                   << value.typeName();
        return value.toUInt();
    }
    if (signature == u"s"_s && value.metaType() != QMetaType::fromType<QString>())
    {
        qWarning() << "PropertyTreeObject: expected QString for signature 's', got"
                   << value.typeName();
        return value.toString();
    }
    if (signature == u"o"_s && value.metaType() != QMetaType::fromType<QDBusObjectPath>())
    {
        qWarning() << "PropertyTreeObject: expected QDBusObjectPath for signature 'o', got"
                   << value.typeName();
        return QVariant::fromValue(QDBusObjectPath(value.toString()));
    }
    return value;
}

QVariantMap coerceBag(InterfaceDescriptor const& descriptor, QVariantMap const& props)
{
    QVariantMap coerced;
    for (auto const& prop : descriptor.properties)
        coerced.insert(prop.name, coerceValue(prop.signature, props.value(prop.name)));
    return coerced;
}

QString stripSubtreeRoot(QString path)
{
    if (path.startsWith(dbus::kActivitySubtreePath))
        path.remove(0, dbus::kActivitySubtreePath.size());
    while (path.startsWith(u'/')) path.remove(0, 1);
    return path;
}

}  // namespace

PropertyTreeObject::PropertyTreeObject(
    InterfaceRegistry const& registry, ObjectTreePublisher const& publisher,
    QDBusConnection connection, QObject* parent
)
    : QDBusVirtualObject(parent),
      m_registry(registry),
      m_publisher(publisher),
      m_connection(std::move(connection))
{
}

QString PropertyTreeObject::introspectionXmlFor(QString const& subPath) const
{
    QString const path = stripSubtreeRoot(subPath);

    if (path.isEmpty())
    {
        // The /activity parent node: lists "current" only while an activity is
        // live. Nothing else is served under this subtree yet.
        bool const live = m_publisher.resolve(dbus::kActivityObjectPath).has_value();
        QString xml = u"<node>\n"_s;
        if (live)
            xml += u"  <node name=\"current\"/>\n"_s;
        xml += u"</node>\n"_s;
        return xml;
    }

    QString const fullPath = dbus::kActivitySubtreePath + u'/' + path;
    auto const snapshot = m_publisher.resolve(fullPath);
    if (!snapshot)
        return u"<node>\n</node>\n"_s;

    QString xml = u"<node>\n"_s;
    for (auto const& [interfaceName, props] : *snapshot)
    {
        Q_UNUSED(props);
        if (auto const* descriptor = m_registry.find(interfaceName))
            xml += descriptor->xml;
    }
    // Standard interfaces Qt's generated adaptor machinery contributes to
    // every classic object's introspection; a QDBusVirtualObject gets none of
    // that for free, so we serve them from data/dbus-standard-interfaces.xml.
    xml += m_registry.standardInterfacesXml();
    xml += u"</node>\n"_s;
    return xml;
}

QString PropertyTreeObject::introspect(QString const& path) const
{
    return introspectionXmlFor(path);
}

bool PropertyTreeObject::handleMessage(
    QDBusMessage const& message, QDBusConnection const& connection
)
{
    if (message.interface() == kPropertiesInterface)
        return handleProperties(message, connection);

    if (message.interface() == kIntrospectableInterface)
        return handleIntrospectable(message, connection);

    return false;
}

bool PropertyTreeObject::handleProperties(
    QDBusMessage const& message, QDBusConnection const& connection
)
{
    QString const member = message.member();
    if (member != u"Get"_s && member != u"GetAll"_s && member != u"Set"_s)
        return false;

    QList<QVariant> const args = message.arguments();
    QString const interfaceName = args.value(0).toString();

    auto const snapshot = m_publisher.resolve(message.path());
    if (!snapshot)
    {
        connection.send(
            message.createErrorReply(kUnknownObjectError, u"No such object " + message.path())
        );
        return true;
    }

    QVariantMap const* bag = nullptr;
    for (auto const& [iface, props] : *snapshot)
    {
        if (iface == interfaceName)
        {
            bag = &props;
            break;
        }
    }

    if (!bag)
    {
        connection.send(
            message.createErrorReply(kUnknownInterfaceError, u"No such interface " + interfaceName)
        );
        return true;
    }

    InterfaceDescriptor const* descriptor = m_registry.find(interfaceName);
    // interfaceName came out of the resolver's own snapshot, so the registry
    // must know its shape.
    Q_ASSERT(descriptor);

    if (member == u"GetAll"_s)
    {
        connection.send(
            message.createReply(QVariantList{QVariant::fromValue(coerceBag(*descriptor, *bag))})
        );
        return true;
    }

    QString const propertyName = args.value(1).toString();
    PropertyDescriptor const* property = descriptor->property(propertyName);
    if (!property)
    {
        connection.send(
            message.createErrorReply(kUnknownPropertyError, u"No such property " + propertyName)
        );
        return true;
    }

    if (member == u"Set"_s)
    {
        connection.send(message.createErrorReply(
            kPropertyReadOnlyError, u"Property " + propertyName + u" is read-only"
        ));
        return true;
    }

    QVariant const value = coerceValue(property->signature, bag->value(propertyName));
    connection.send(message.createReply(QVariantList{QVariant::fromValue(QDBusVariant(value))}));
    return true;
}

bool PropertyTreeObject::handleIntrospectable(
    QDBusMessage const& message, QDBusConnection const& connection
)
{
    if (message.member() != u"Introspect"_s)
        return false;

    connection.send(
        message.createReply(QVariantList{introspectionXmlFor(stripSubtreeRoot(message.path()))})
    );
    return true;
}

void PropertyTreeObject::emitPropertiesChanged(
    QString const& path, QString const& interfaceName, QVariantMap const& changedProperties
)
{
    m_connection.send(buildPropertiesChangedMessage(path, interfaceName, changedProperties));
}

QDBusMessage PropertyTreeObject::buildPropertiesChangedMessage(
    QString const& path, QString const& interfaceName, QVariantMap const& changedProperties
)
{
    QDBusMessage signal =
        QDBusMessage::createSignal(path, kPropertiesInterface, u"PropertiesChanged"_s);
    signal << interfaceName;
    signal << QVariant::fromValue(changedProperties);
    signal << QVariant::fromValue(QStringList{});  // invalidated_properties: always empty
    return signal;
}
