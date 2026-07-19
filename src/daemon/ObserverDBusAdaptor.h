#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

#include "DBusConstants.h"

class ObserverService;

class ObserverDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", CONSTELLAR_DBUS_INTERFACE_NAME)

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
