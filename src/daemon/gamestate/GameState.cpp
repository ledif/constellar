#include "GameState.h"

GameState::GameState(QObject* parent) : QObject(parent) {}

QVariantMap GameState::activity() const
{
    return m_activity;
}

QVariantMap GameState::zone() const
{
    return m_zone;
}

void GameState::setActivity(QVariantMap const& activity)
{
    if (m_activity == activity)
        return;
    m_activity = activity;
    Q_EMIT activityChanged(m_activity);
}

void GameState::setZone(QVariantMap const& zone)
{
    if (m_zone == zone)
        return;
    m_zone = zone;
    Q_EMIT zoneChanged(m_zone);
}

void GameState::endActivity(QVariantMap const& endedActivity)
{
    Q_EMIT activityEnded(endedActivity);
    setActivity({});
}
