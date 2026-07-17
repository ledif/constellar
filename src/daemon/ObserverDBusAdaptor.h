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
//
// QDBusAbstractAdaptor does not emit org.freedesktop.DBus.Properties
// .PropertiesChanged automatically -- Activity/Zone changes are relayed by
// hand via emitPropertiesChanged() (ADR-012).
class ObserverDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.ledif.constellar.Observer")

    Q_PROPERTY(QVariantMap Activity READ activity)
    Q_PROPERTY(QVariantMap Zone READ zone)

  public:
    explicit ObserverDBusAdaptor(ObserverService* service);

    QVariantMap activity() const;
    QVariantMap zone() const;

  public Q_SLOTS:
    void Pause();
    void Resume();
    void StartManualRecording();
    void StopManualRecording();
    void ReloadConfig();
    QStringList ListRecordings(QVariantMap const& filter);
    void DeleteRecording(QString const& id);
    QVariantMap GetConfig();
    void SetConfig(QVariantMap const& config);

  Q_SIGNALS:
    void ActivityEnded(QVariantMap const& activity);
    void Error(QString const& code, QString const& message);

  private:
    void emitPropertiesChanged(QString const& name, QVariant const& value);

    ObserverService* m_service;
};
