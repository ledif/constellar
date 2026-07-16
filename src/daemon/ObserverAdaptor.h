#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

class ObserverService;

// DBus adaptor for io.github.ledif.constellar.Observer. Hand-written rather
// than qdbusxml2cpp-generated: the generated skeleton needs its method
// bodies filled in by hand anyway, so we skip the generation step on the
// server side and keep data/io.github.ledif.constellar.xml as the
// documented source of truth. The client-side proxy (src/common) *is*
// generated, since that code is fully mechanical.
class ObserverAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.ledif.constellar.Observer")

    Q_PROPERTY(QString State READ state)
    Q_PROPERTY(bool WowActive READ wowActive)
    Q_PROPERTY(QString ActiveCapture READ activeCapture)

  public:
    explicit ObserverAdaptor(ObserverService *service);

    QString state() const;
    bool wowActive() const;
    QString activeCapture() const;

  public Q_SLOTS:
    QVariantMap Status();
    void Pause();
    void Resume();
    void StartManualRecording();
    void StopManualRecording();
    void ReloadConfig();
    QStringList ListRecordings(const QVariantMap &filter);
    void DeleteRecording(const QString &id);
    QVariantMap GetConfig();
    void SetConfig(const QVariantMap &config);

  Q_SIGNALS:
    void EncounterDetected(int encounterId, const QString &encounterName, const QString &difficulty,
                           const QString &startTime);
    void EncounterEnded(int encounterId, const QString &encounterName, bool success,
                        const QString &stopTime);
    void DungeonDetected(int zoneId, int mapId, int keystoneLevel, const QString &startTime);
    void DungeonEnded(int mapId, int keystoneLevel, bool success, int durationMs,
                      const QString &stopTime);
    void StateChanged(const QString &state);
    void Error(const QString &code, const QString &message);

  private:
    ObserverService *m_service;
};
