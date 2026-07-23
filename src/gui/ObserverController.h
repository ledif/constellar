#pragma once

#include <qqmlintegration.h>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantMap>

#include "ActivityModel.h"
#include "observerproxy.h"

class ObserverController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connected READ connected NOTIFY stateChanged)
    Q_PROPERTY(QString zoneText READ zoneText NOTIFY stateChanged)
    Q_PROPERTY(ActivityModel* eventLog READ eventLog CONSTANT)
    Q_PROPERTY(qint64 now READ now NOTIFY nowChanged)

  public:
    explicit ObserverController(QObject* parent = nullptr);

    bool connected() const
    {
        return m_manager.isValid();
    }

    QString zoneText() const
    {
        return m_zoneText;
    }

    ActivityModel* eventLog()
    {
        return &m_eventLog;
    }

    qint64 now() const
    {
        return m_now;
    }

    Q_INVOKABLE void refresh();

    Q_INVOKABLE QString relativeTime(qint64 epochMs) const;
    Q_INVOKABLE QString elapsed(qint64 startMs) const;
    Q_INVOKABLE QString durationText(qint64 durationMs) const;

  Q_SIGNALS:
    void stateChanged();
    void nowChanged();

  private:
    void onActivityEnded(QDBusObjectPath const& path, QVariantMap const& activityMap);

    ObserverProxy m_manager;
    QTimer m_pollTimer;
    ActivityModel m_eventLog;
    QVariantMap m_previousActivity;
    QString m_zoneText;
    qint64 m_now = 0;
};
