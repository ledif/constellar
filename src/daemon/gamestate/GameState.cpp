#include "GameState.h"

GameState::GameState(QObject* parent) : QObject(parent) {}

QVariantMap GameState::activity() const
{
    return m_activity;
}

QVariantMap GameState::location() const
{
    return m_location.toVariantMap();
}

void GameState::setActivity(QVariantMap const& activity)
{
    if (m_activity == activity)
        return;

    m_activity = activity;
    Q_EMIT activityChanged(m_activity);
}

void GameState::setUiMap(UiMap const& uiMap)
{
    QVariantMap const before = m_location.toVariantMap();
    m_location.setUiMap(uiMap);
    QVariantMap const after = m_location.toVariantMap();
    if (before == after)
        return;

    Q_EMIT locationChanged(after);
}

void GameState::setZone(Zone const& zone)
{
    QVariantMap const before = m_location.toVariantMap();
    m_location.setZone(zone);
    QVariantMap const after = m_location.toVariantMap();
    if (before == after)
        return;

    Q_EMIT locationChanged(after);
}

void GameState::endActivity(QVariantMap const& endedActivity)
{
    Q_EMIT activityEnded(endedActivity);
    setActivity({});
}
