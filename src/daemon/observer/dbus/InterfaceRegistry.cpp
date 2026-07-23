#include "InterfaceRegistry.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace Qt::StringLiterals;

namespace
{

QString const kContractResourcePath = u":/dbus/dev.ulduar.Constellar1.xml"_s;
QString const kStandardResourcePath = u":/dbus/dbus-standard-interfaces.xml"_s;

QString readResource(QString const& resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("InterfaceRegistry: failed to open embedded resource %s", qPrintable(resourcePath));
    return QString::fromUtf8(file.readAll());
}

// Shift a serialized <interface> fragment one level right
QString indentFragment(QString const& fragment)
{
    QString indented;
    for (auto const& line : fragment.split(u'\n')) indented += u"  "_s + line + u'\n';
    return indented;
}

}  // namespace

PropertyDescriptor const* InterfaceDescriptor::property(QString const& propertyName) const
{
    for (auto const& prop : properties)
        if (prop.name == propertyName)
            return &prop;
    return nullptr;
}

InterfaceRegistry::InterfaceRegistry()
{
    // We embeded our raw D-Bus XML specs into this lib, so let's pull em out
    Q_INIT_RESOURCE(constellar_observer_dbus_xml);

    parse(readResource(kContractResourcePath), /*standard=*/false);
    parse(readResource(kStandardResourcePath), /*standard=*/true);
}

InterfaceDescriptor const* InterfaceRegistry::find(QString const& interfaceName) const
{
    auto const it = m_interfaces.constFind(interfaceName);
    return it == m_interfaces.constEnd() ? nullptr : &it.value();
}

void InterfaceRegistry::parse(QString const& xml, bool standard)
{
    QXmlStreamReader reader(xml);

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isStartElement() && reader.name() == u"interface"_s)
        {
            InterfaceDescriptor descriptor = readInterface(reader);
            if (standard)
                m_standardInterfacesXml += descriptor.xml;
            else
                m_interfaces.insert(descriptor.name, descriptor);
        }
    }

    if (reader.hasError())
    {
        qFatal(
            "InterfaceRegistry: failed to parse interface XML: %s", qPrintable(reader.errorString())
        );
    }
}

// Collects the property descriptors and re-serializes the <interface> subtree
// in one pass over the same tokens, so the introspection fragment and the
// descriptors can never disagree.
InterfaceDescriptor InterfaceRegistry::readInterface(QXmlStreamReader& reader)
{
    // reader is positioned on the <interface> start element
    InterfaceDescriptor descriptor;
    descriptor.name = reader.attributes().value(u"name"_s).toString();

    QString fragment;
    QXmlStreamWriter writer(&fragment);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(2);

    int depth = 0;
    while (!reader.atEnd())
    {
        // Drop the source file's inter-element whitespace; the writer's
        // auto-formatting owns the fragment's layout.
        if (reader.isCharacters() && reader.isWhitespace())
        {
            reader.readNext();
            continue;
        }

        if (reader.isStartElement())
        {
            ++depth;
            if (reader.name() == u"property"_s)
            {
                auto const attrs = reader.attributes();
                QString const access = attrs.value(u"access"_s).toString();
                descriptor.properties.append(
                    PropertyDescriptor{
                        .name = attrs.value(u"name"_s).toString(),
                        .signature = attrs.value(u"type"_s).toString(),
                        .readOnly = access != u"readwrite"_s && access != u"write"_s,
                    }
                );
            }
        }

        writer.writeCurrentToken(reader);

        if (reader.isEndElement() && --depth == 0)
            break;

        reader.readNext();
    }

    descriptor.xml = indentFragment(fragment);
    return descriptor;
}
