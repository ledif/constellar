#include "ObserverAdaptor.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include "DBusConstants.h"
#include "GameState.h"
#include "ObserverService.h"

using namespace Qt::StringLiterals;

ObserverAdaptor::ObserverAdaptor(ObserverService* service)
    : QDBusAbstractAdaptor(service), m_service(service)
{
    setAutoRelaySignals(true);
    GameState const& gameState = m_service->gameState();
    connect(
        &gameState, &GameState::activityChanged, this,
        [this](QVariantMap const& activity) { emitPropertiesChanged(u"Activity"_s, activity); }
    );
    connect(
        &gameState, &GameState::zoneChanged, this,
        [this](QVariantMap const& zone) { emitPropertiesChanged(u"Zone"_s, zone); }
    );
    connect(&gameState, &GameState::activityEnded, this, &ObserverAdaptor::ActivityEnded);
}

QVariantMap ObserverAdaptor::activity() const
{
    return m_service->gameState().activity();
}

QVariantMap ObserverAdaptor::zone() const
{
    return m_service->gameState().zone();
}

void ObserverAdaptor::emitPropertiesChanged(QString const& name, QVariant const& value)
{
    QDBusMessage signal = QDBusMessage::createSignal(
        constellar::dbus::kObjectPath, u"org.freedesktop.DBus.Properties"_s, u"PropertiesChanged"_s
    );
    signal << u"io.github.ledif.constellar.Observer"_s << QVariantMap{{name, value}}
           << QStringList{};
    QDBusConnection::sessionBus().send(signal);
}

void ObserverAdaptor::Pause()
{
    // Phase 0 stub.
}

void ObserverAdaptor::Resume()
{
    // Phase 0 stub.
}

void ObserverAdaptor::StartManualRecording()
{
    // Phase 0 stub.
}

void ObserverAdaptor::StopManualRecording()
{
    // Phase 0 stub.
}

void ObserverAdaptor::ReloadConfig()
{
    // Phase 0 stub.
}

QStringList ObserverAdaptor::ListRecordings(QVariantMap const& /*filter*/)
{
    return {};
}

void ObserverAdaptor::DeleteRecording(QString const& /*id*/)
{
    // Phase 0 stub.
}

QVariantMap ObserverAdaptor::GetConfig()
{
    return {};
}

void ObserverAdaptor::SetConfig(QVariantMap const& /*config*/)
{
    // Phase 0 stub.
}
