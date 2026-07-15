#include "ManagerAdaptor.h"

#include "ManagerService.h"

ManagerAdaptor::ManagerAdaptor(ManagerService *service)
    : QDBusAbstractAdaptor(service), m_service(service) {
    setAutoRelaySignals(true);
    connect(service, &ManagerService::stateChanged, this, &ManagerAdaptor::StateChanged);
    connect(service, &ManagerService::encounterDetected, this, &ManagerAdaptor::EncounterDetected);
    connect(service, &ManagerService::encounterEnded, this, &ManagerAdaptor::EncounterEnded);
    connect(service, &ManagerService::dungeonDetected, this, &ManagerAdaptor::DungeonDetected);
    connect(service, &ManagerService::dungeonEnded, this, &ManagerAdaptor::DungeonEnded);
}

QString ManagerAdaptor::state() const {
    return m_service->state();
}

bool ManagerAdaptor::wowActive() const {
    return m_service->wowActive();
}

QString ManagerAdaptor::activeCapture() const {
    return m_service->activeCapture();
}

QVariantMap ManagerAdaptor::Status() {
    QVariantMap status;
    status[QStringLiteral("state")] = m_service->state();
    status[QStringLiteral("wowActive")] = m_service->wowActive();
    status[QStringLiteral("activeCapture")] = m_service->activeCapture();
    return status;
}

void ManagerAdaptor::Pause() {
    // Phase 0 stub.
}

void ManagerAdaptor::Resume() {
    // Phase 0 stub.
}

void ManagerAdaptor::StartManualRecording() {
    // Phase 0 stub.
}

void ManagerAdaptor::StopManualRecording() {
    // Phase 0 stub.
}

void ManagerAdaptor::ReloadConfig() {
    // Phase 0 stub.
}

QStringList ManagerAdaptor::ListRecordings(const QVariantMap & /*filter*/) {
    return {};
}

void ManagerAdaptor::DeleteRecording(const QString & /*id*/) {
    // Phase 0 stub.
}

QVariantMap ManagerAdaptor::GetConfig() {
    return {};
}

void ManagerAdaptor::SetConfig(const QVariantMap & /*config*/) {
    // Phase 0 stub.
}
