#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QString>

#include "DBusConstants.h"

class CurrentActivityObject;

// dev.ulduar.Constellar1.Activity -- base facts present on every activity object
// (TASK-008). Read-only projection of the CurrentActivityObject's bag.
class ActivityDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", CONSTELLAR_ACTIVITY_INTERFACE_NAME)

    Q_PROPERTY(QString Type READ type)
    Q_PROPERTY(qint64 StartTime READ startTime)
    Q_PROPERTY(qint64 StopTime READ stopTime)
    Q_PROPERTY(QString Outcome READ outcome)
    Q_PROPERTY(QDBusObjectPath Recording READ recording)

  public:
    explicit ActivityDBusAdaptor(CurrentActivityObject* activity);

    QString type() const;
    qint64 startTime() const;
    qint64 stopTime() const;
    QString outcome() const;
    QDBusObjectPath recording() const;

  private:
    CurrentActivityObject* m_activity;
};
