#include "ManagerService.h"

ManagerService::ManagerService(QObject *parent) : QObject(parent) {}

QString ManagerService::state() const {
    return m_state;
}

bool ManagerService::wowActive() const {
    return m_wowActive;
}

QString ManagerService::activeCapture() const {
    return m_activeCapture;
}
