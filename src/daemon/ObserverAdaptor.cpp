#include "ObserverAdaptor.h"

#include "ObserverService.h"

ObserverAdaptor::ObserverAdaptor(ObserverService *service)
    : QDBusAbstractAdaptor(service), m_service(service) {
    setAutoRelaySignals(true);
    connect(service, &ObserverService::stateChanged, this, &ObserverAdaptor::StateChanged);
    connect(service, &ObserverService::encounterDetected, this,
            &ObserverAdaptor::EncounterDetected);
    connect(service, &ObserverService::encounterEnded, this, &ObserverAdaptor::EncounterEnded);
    connect(service, &ObserverService::dungeonDetected, this, &ObserverAdaptor::DungeonDetected);
    connect(service, &ObserverService::dungeonEnded, this, &ObserverAdaptor::DungeonEnded);
    connect(service, &ObserverService::zoneChanged, this, &ObserverAdaptor::ZoneChanged);
}

QString ObserverAdaptor::state() const {
    return m_service->state();
}

bool ObserverAdaptor::wowActive() const {
    return m_service->wowActive();
}

QString ObserverAdaptor::activeCapture() const {
    return m_service->activeCapture();
}

QVariantMap ObserverAdaptor::Status() {
    QVariantMap status;
    status[QStringLiteral("state")] = m_service->state();
    status[QStringLiteral("wowActive")] = m_service->wowActive();
    status[QStringLiteral("activeCapture")] = m_service->activeCapture();
    return status;
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
