#include "GameState.h"

GameState::GameState(QObject *parent) : QObject(parent) {}

QVariantMap GameState::activity() const {
    return m_activity;
}

QVariantMap GameState::zone() const {
    return m_zone;
}

void GameState::setActivity(const QVariantMap &activity) {
    if (m_activity == activity) {
        return;
    }
    m_activity = activity;
    Q_EMIT activityChanged(m_activity);
}

void GameState::clearActivity() {
    setActivity({});
}

void GameState::setZone(const QVariantMap &zone) {
    if (m_zone == zone) {
        return;
    }
    m_zone = zone;
    Q_EMIT zoneChanged(m_zone);
}

void GameState::endActivity(const QVariantMap &endedActivity) {
    Q_EMIT activityEnded(endedActivity);
    clearActivity();
}
