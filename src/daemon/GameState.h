#pragma once

#include <QObject>
#include <QVariantMap>

// Single source of truth for the two fact bags ObserverAdaptor publishes as
// DBus properties (ADR-012). ObserverService fills this in from
// RecordingController's decisions; ObserverAdaptor reads it for
// Activity/Zone and relays its signals as PropertiesChanged/ActivityEnded;
// PresencePublisher reads it as a pure projection instead of keeping its own
// shadow state.
class GameState : public QObject
{
    Q_OBJECT

  public:
    explicit GameState(QObject* parent = nullptr);

    QVariantMap activity() const;
    QVariantMap zone() const;

    // No-op (no signal) if unchanged, so PropertiesChanged doesn't storm.
    void setActivity(QVariantMap const& activity);
    void clearActivity();
    void setZone(QVariantMap const& zone);

    // The one genuine edge: emits activityEnded with the outcome bag, then
    // clears Activity. Replaces "build ended bag, emit, then clear" as one
    // call at every call site.
    void endActivity(QVariantMap const& endedActivity);

  Q_SIGNALS:
    void activityChanged(QVariantMap const& activity);
    void zoneChanged(QVariantMap const& zone);
    void activityEnded(QVariantMap const& endedActivity);

  private:
    QVariantMap m_activity;
    QVariantMap m_zone;
};
