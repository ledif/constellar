#include "ObserverDBusAdaptor.h"

#include <utility>

#include <QDBusConnection>
#include <QDBusMessage>

#include "DBusConstants.h"
#include "GameState.h"
#include "ObserverService.h"

using namespace Qt::StringLiterals;

ObserverDBusAdaptor::ObserverDBusAdaptor(ObserverService* service, QDBusConnection connection)
    : QDBusAbstractAdaptor(service), m_service(service), m_connection(std::move(connection))
{
    setAutoRelaySignals(true);
    GameState const& gameState = m_service->gameState();

    connect(
        &gameState, &GameState::activityChanged, this,
        [this](QVariantMap const& activity) { emitPropertiesChanged(u"Activity"_s, activity); }
    );

    connect(
        &gameState, &GameState::locationChanged, this,
        [this](QVariantMap const& location) { emitPropertiesChanged(u"Location"_s, location); }
    );

    connect(
        &gameState, &GameState::activityEnded, this, [this](QVariantMap const& activity)
        { Q_EMIT ActivityEnded(QDBusObjectPath(constellar::dbus::kActivityObjectPath), activity); }
    );
}

QVariantMap ObserverDBusAdaptor::activity() const
{
    return m_service->gameState().activity();
}

QVariantMap ObserverDBusAdaptor::location() const
{
    return m_service->gameState().location();
}

void ObserverDBusAdaptor::emitPropertiesChanged(QString const& name, QVariant const& value)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        constellar::dbus::kObjectPath, u"org.freedesktop.DBus.Properties"_s, u"PropertiesChanged"_s
    );

    signal << constellar::dbus::kInterfaceName << QVariantMap{{name, value}} << QStringList{};

    m_connection.send(signal);
}

void ObserverDBusAdaptor::Pause()
{
    // Phase 0 stub.
}

void ObserverDBusAdaptor::Resume()
{
    // Phase 0 stub.
}

void ObserverDBusAdaptor::StartManualRecording()
{
    // Phase 0 stub.
}

void ObserverDBusAdaptor::StopManualRecording()
{
    // Phase 0 stub.
}

void ObserverDBusAdaptor::ReloadConfig()
{
    // Phase 0 stub.
}

QStringList ObserverDBusAdaptor::ListRecordings(QVariantMap const& /*filter*/)
{
    return {};
}

void ObserverDBusAdaptor::DeleteRecording(QString const& /*id*/)
{
    // Phase 0 stub.
}

QVariantMap ObserverDBusAdaptor::GetConfig()
{
    return {};
}

void ObserverDBusAdaptor::SetConfig(QVariantMap const& /*config*/)
{
    // Phase 0 stub.
}
