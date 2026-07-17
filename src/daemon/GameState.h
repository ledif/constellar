#pragma once

#include <QObject>
#include <QVariantMap>

// Single source of truth for the two fact bags ObserverAdaptor publishes as
// DBus properties (ADR-012). ObserverService fills this in from
// RecordingController's decisions; ObserverAdaptor reads it for
// Activity/Zone and relays its signals as PropertiesChanged/ActivityEnded;
// PresencePublisher reads it as a pure projection instead of keeping its own
// shadow state.
class GameState : public QObject {
    Q_OBJECT

  public:
    explicit GameState(QObject *parent = nullptr);

    QVariantMap activity() const;
    QVariantMap zone() const;

    // No-op (no signal) if unchanged, so PropertiesChanged doesn't storm.
    void setActivity(const QVariantMap &activity);
    void clearActivity();
    void setZone(const QVariantMap &zone);

    // The one genuine edge: emits activityEnded with the outcome bag, then
    // clears Activity. Replaces "build ended bag, emit, then clear" as one
    // call at every call site.
    void endActivity(const QVariantMap &endedActivity);

  Q_SIGNALS:
    void activityChanged(const QVariantMap &activity);
    void zoneChanged(const QVariantMap &zone);
    void activityEnded(const QVariantMap &endedActivity);

  private:
    QVariantMap m_activity;
    QVariantMap m_zone;
};
