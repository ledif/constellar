#pragma once

#include <qqmlintegration.h>
#include <QDBusConnection>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>

#include "EventLogModel.h"
#include "observerproxy.h"

class ObserverController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connected READ connected NOTIFY stateChanged)
    Q_PROPERTY(QString activityText READ activityText NOTIFY stateChanged)
    Q_PROPERTY(QString zoneText READ zoneText NOTIFY stateChanged)
    Q_PROPERTY(EventLogModel* eventLog READ eventLog CONSTANT)

  public:
    explicit ObserverController(QObject* parent = nullptr);

    bool connected() const
    {
        return m_manager.isValid();
    }

    QString activityText() const
    {
        return m_activityText;
    }

    QString zoneText() const
    {
        return m_zoneText;
    }

    EventLogModel* eventLog()
    {
        return &m_eventLog;
    }

    Q_INVOKABLE void refresh();

  Q_SIGNALS:
    void stateChanged();

  private:
    void onActivityEnded(QVariantMap const& activityMap);
    static QString shortTime(qint64 epochMs);

    ObserverProxy m_manager;
    QTimer m_pollTimer;
    EventLogModel m_eventLog;
    QVariantMap m_previousActivity;
    QString m_activityText;
    QString m_zoneText;
};
