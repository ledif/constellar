#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QString>

#include "DBusConstants.h"

class ActivityObject;

// dev.ulduar.Constellar1.Activity
class ActivityDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", CONSTELLAR_DBUS_ACTIVITY_INTERFACE_NAME)

    Q_PROPERTY(QString Type READ type)
    Q_PROPERTY(qint64 StartTime READ startTime)
    Q_PROPERTY(qint64 StopTime READ stopTime)
    Q_PROPERTY(QString Outcome READ outcome)
    Q_PROPERTY(QDBusObjectPath Recording READ recording)

  public:
    explicit ActivityDBusAdaptor(ActivityObject* activity);

    QString type() const;
    qint64 startTime() const;
    qint64 stopTime() const;
    QString outcome() const;
    QDBusObjectPath recording() const;

  private:
    ActivityObject* m_activity;
};
