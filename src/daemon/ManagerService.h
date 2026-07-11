#pragma once

#include <QObject>
#include <QString>

// Phase 0 stub: owns the daemon's observable state and will grow into the
// real coordinator (LogWatcher, RecordingController, ObsEngine, stores) in
// later phases. For now it just tracks/report a fixed idle state so the
// DBus round trip (wowcap status -> daemon -> back) can be exercised.
class ManagerService : public QObject {
    Q_OBJECT

  public:
    explicit ManagerService(QObject *parent = nullptr);

    QString state() const;
    bool wowActive() const;
    QString activeCapture() const;

  Q_SIGNALS:
    void stateChanged(const QString &state);

  private:
    QString m_state = QStringLiteral("idle");
    bool m_wowActive = false;
    QString m_activeCapture = QStringLiteral("none");
};
