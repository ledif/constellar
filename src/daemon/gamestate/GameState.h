#pragma once

#include <QObject>
#include <QVariantMap>

#include "Location.h"

class GameState : public QObject
{
    Q_OBJECT

  public:
    explicit GameState(QObject* parent = nullptr);

    QVariantMap activity() const;
    QVariantMap location() const;

    void setActivity(QVariantMap const& activity);
    void endActivity(QVariantMap const& endedActivity);
    void setUiMap(UiMap const& uiMap);
    void setZone(Zone const& zone);

  Q_SIGNALS:
    void activityChanged(QVariantMap const& activity);
    void activityEnded(QVariantMap const& endedActivity);
    void locationChanged(QVariantMap const& location);

  private:
    QVariantMap m_activity;
    Location m_location;
};
