#include "ObserverAdaptor.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include "DBusConstants.h"
#include "GameState.h"
#include "ObserverService.h"

ObserverAdaptor::ObserverAdaptor(ObserverService *service)
    : QDBusAbstractAdaptor(service), m_service(service) {
    setAutoRelaySignals(true);
    GameState &gameState = m_service->gameState();
    connect(&gameState, &GameState::activityChanged, this, [this](const QVariantMap &activity) {
        emitPropertiesChanged(QStringLiteral("Activity"), activity);
    });
    connect(&gameState, &GameState::zoneChanged, this, [this](const QVariantMap &zone) {
        emitPropertiesChanged(QStringLiteral("Zone"), zone);
    });
    connect(&gameState, &GameState::activityEnded, this, &ObserverAdaptor::ActivityEnded);
}

QVariantMap ObserverAdaptor::activity() const {
    return m_service->gameState().activity();
}

QVariantMap ObserverAdaptor::zone() const {
    return m_service->gameState().zone();
}

void ObserverAdaptor::emitPropertiesChanged(const QString &name, const QVariant &value) {
    QDBusMessage signal = QDBusMessage::createSignal(
        constellar::dbus::kObjectPath, QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"));
    signal << QStringLiteral("io.github.ledif.constellar.Observer") << QVariantMap{{name, value}}
           << QStringList{};
    QDBusConnection::sessionBus().send(signal);
}

void ObserverAdaptor::Pause() {
    // Phase 0 stub.
}

void ObserverAdaptor::Resume() {
    // Phase 0 stub.
}

void ObserverAdaptor::StartManualRecording() {
    // Phase 0 stub.
}

void ObserverAdaptor::StopManualRecording() {
    // Phase 0 stub.
}

void ObserverAdaptor::ReloadConfig() {
    // Phase 0 stub.
}

QStringList ObserverAdaptor::ListRecordings(const QVariantMap & /*filter*/) {
    return {};
}

void ObserverAdaptor::DeleteRecording(const QString & /*id*/) {
    // Phase 0 stub.
}

QVariantMap ObserverAdaptor::GetConfig() {
    return {};
}

void ObserverAdaptor::SetConfig(const QVariantMap & /*config*/) {
    // Phase 0 stub.
}
