#pragma once

#include <QObject>
#include <QVariantMap>

class GameState : public QObject
{
    Q_OBJECT

  public:
    explicit GameState(QObject* parent = nullptr);

    QVariantMap activity() const;
    QVariantMap zone() const;

    void setActivity(QVariantMap const& activity);
    void endActivity(QVariantMap const& endedActivity);
    void setZone(QVariantMap const& zone);

  Q_SIGNALS:
    void activityChanged(QVariantMap const& activity);
    void activityEnded(QVariantMap const& endedActivity);
    void zoneChanged(QVariantMap const& zone);

  private:
    QVariantMap m_activity;
    QVariantMap m_zone;
};
