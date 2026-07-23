#include "InterfaceRegistry.h"

#include <QFile>
#include <QXmlStreamReader>

using namespace Qt::StringLiterals;

namespace
{

QString const kResourcePath = u":/dbus/dev.ulduar.Constellar1.xml"_s;
QString const kInterfaceOpenMarker = u"<interface"_s;
QString const kInterfaceCloseTag = u"</interface>"_s;

// The canonical XML is hand-authored (ADR-016) and holds each <interface> as a
// single, non-nested block -- slicing it out verbatim is simpler and more
// faithful than re-serializing a hand-rolled parse tree.
QString extractInterfaceXml(QString const& xml, QString const& interfaceName)
{
    QString const nameMarker = u"name=\"" + interfaceName + u'"';

    int searchFrom = 0;
    while (true)
    {
        int const nameIdx = xml.indexOf(nameMarker, searchFrom);
        if (nameIdx < 0)
            return {};

        int const tagStart = xml.lastIndexOf(kInterfaceOpenMarker, nameIdx);
        int const tagEnd = tagStart < 0 ? -1 : xml.indexOf(u'>', tagStart);

        // Only accept the match if it's this <interface ...> tag's own "name"
        // attribute, i.e. nameIdx sits before that tag's closing '>'.
        if (tagStart >= 0 && tagEnd >= 0 && nameIdx < tagEnd)
        {
            int const closeIdx = xml.indexOf(kInterfaceCloseTag, tagEnd);
            if (closeIdx < 0)
                return {};

            return xml.mid(tagStart, closeIdx + kInterfaceCloseTag.size() - tagStart).trimmed();
        }

        searchFrom = nameIdx + nameMarker.size();
    }
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
    // The resource is compiled into the static constellar-observer library, whose
    // translation units the linker is otherwise free to drop; this forces it in.
    Q_INIT_RESOURCE(observer_dbus_xml);

    QFile file(kResourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        qFatal("InterfaceRegistry: failed to open embedded resource %s", qPrintable(kResourcePath));

    parse(QString::fromUtf8(file.readAll()));
}

InterfaceDescriptor const* InterfaceRegistry::find(QString const& interfaceName) const
{
    auto const it = m_interfaces.constFind(interfaceName);
    return it == m_interfaces.constEnd() ? nullptr : &it.value();
}

void InterfaceRegistry::parse(QString const& xml)
{
    QXmlStreamReader reader(xml);

    InterfaceDescriptor current;
    bool inInterface = false;

    while (!reader.atEnd())
    {
        reader.readNext();

        if (reader.isStartElement())
        {
            if (reader.name() == u"interface"_s)
            {
                current = InterfaceDescriptor{};
                current.name = reader.attributes().value(u"name"_s).toString();
                inInterface = true;
            }
            else if (inInterface && reader.name() == u"property"_s)
            {
                auto const attrs = reader.attributes();
                QString const access = attrs.value(u"access"_s).toString();
                current.properties.append(
                    PropertyDescriptor{
                        .name = attrs.value(u"name"_s).toString(),
                        .signature = attrs.value(u"type"_s).toString(),
                        .readOnly = access != u"readwrite"_s && access != u"write"_s,
                    }
                );
            }
        }
        else if (reader.isEndElement() && reader.name() == u"interface"_s)
        {
            current.xml = extractInterfaceXml(xml, current.name);
            m_interfaces.insert(current.name, current);
            inInterface = false;
        }
    }

    if (reader.hasError())
    {
        qFatal(
            "InterfaceRegistry: failed to parse dev.ulduar.Constellar1.xml: %s",
            qPrintable(reader.errorString())
        );
    }
}
