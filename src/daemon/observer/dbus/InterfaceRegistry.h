#pragma once

#include <QHash>
#include <QString>
#include <QVector>

class QXmlStreamReader;

// One property of a D-Bus interface as declared in the canonical XML.
struct PropertyDescriptor
{
    QString name;
    QString signature;  // D-Bus type signature, e.g. "s", "x", "u", "o"
    bool readOnly = true;
};

// One <interface> from the XML
struct InterfaceDescriptor
{
    QString name;
    QVector<PropertyDescriptor> properties;
    QString xml;

    PropertyDescriptor const* property(QString const& propertyName) const;
};

// Parses and holds our embeded XML files
class InterfaceRegistry
{
  public:
    InterfaceRegistry();

    InterfaceDescriptor const* find(QString const& interfaceName) const;

    QString const& standardInterfacesXml() const
    {
        return m_standardInterfacesXml;
    }

  private:
    void parse(QString const& xml, bool standard);
    InterfaceDescriptor readInterface(QXmlStreamReader& reader);

    QHash<QString, InterfaceDescriptor> m_interfaces;
    QString m_standardInterfacesXml;
};
