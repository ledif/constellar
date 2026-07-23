#pragma once

#include <QHash>
#include <QString>
#include <QVector>

// One property of a D-Bus interface, as declared in the canonical XML.
struct PropertyDescriptor
{
    QString name;
    QString signature;  // D-Bus type signature, e.g. "s", "x", "u", "o"
    bool readOnly = true;
};

// One interface's shape: its ordered properties, plus the exact <interface>...
// </interface> subtree from the canonical XML, retained verbatim for introspection.
struct InterfaceDescriptor
{
    QString name;
    QVector<PropertyDescriptor> properties;
    QString xml;

    PropertyDescriptor const* property(QString const& propertyName) const;
};

// Parses data/dev.ulduar.Constellar1.xml (embedded as a Qt resource) once at
// startup. A malformed XML is a fatal startup error -- the file is the wire
// contract and a daemon that can't read it can't serve anything correctly.
class InterfaceRegistry
{
  public:
    InterfaceRegistry();

    InterfaceDescriptor const* find(QString const& interfaceName) const;

  private:
    void parse(QString const& xml);

    QHash<QString, InterfaceDescriptor> m_interfaces;
};
