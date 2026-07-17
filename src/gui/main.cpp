#include <QApplication>
#include <QDBusConnection>
#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QWidget>

#include "Activity.h"
#include "ActivityKeys.h"
#include "DBusConstants.h"
#include "Zone.h"
#include "observerproxy.h"

using namespace Qt::StringLiterals;

namespace keys = constellar::keys;

// Still a stub label (ADR-012) -- polls Activity/Zone on a timer rather than
// wiring up org.freedesktop.DBus.Properties.PropertiesChanged, since nothing
// here needs sub-second latency yet.
class StatusWindow : public QWidget
{
  public:
    StatusWindow()
        : m_manager(
              constellar::dbus::kServiceName, constellar::dbus::kObjectPath,
              QDBusConnection::sessionBus()
          )
    {
        setWindowTitle(u"Constellar"_s);
        resize(480, 360);

        auto* layout = new QVBoxLayout(this);
        m_label = new QLabel(u"connecting..."_s, this);
        layout->addWidget(m_label);

        m_events = new QListWidget(this);
        layout->addWidget(m_events);

        connect(&m_manager, &ObserverProxy::ActivityEnded, this, &StatusWindow::onActivityEnded);

        auto* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &StatusWindow::refresh);
        timer->start(1000);
        refresh();
    }

  private:
    void refresh()
    {
        if (!m_manager.isValid())
        {
            m_label->setText(u"constellard not reachable"_s);
            return;
        }

        QVariantMap const activityMap = m_manager.property("Activity").toMap();
        Activity const activity = Activity::fromVariantMap(activityMap);
        Zone const zone = Zone::fromVariantMap(m_manager.property("Zone").toMap());

        if (m_previousActivity.isEmpty() && !activityMap.isEmpty())
        {
            m_events->addItem(u"▶ %1"_s.arg(activity.toString()));
            m_events->scrollToBottom();
        }
        m_previousActivity = activityMap;

        m_label->setText(u"activity: %1\nzone: %2"_s.arg(activity.toString(), zone.name()));
    }

    static QString shortTime(qint64 epochMs)
    {
        return QDateTime::fromMSecsSinceEpoch(epochMs).toString(u"HH:mm:ss"_s);
    }

    void onActivityEnded(QVariantMap const& activityMap)
    {
        Activity const activity = Activity::fromVariantMap(activityMap);
        bool const success = activityMap.value(QString::fromLatin1(keys::kSuccess)).toBool();
        qint64 const stopTime =
            activityMap.value(QString::fromLatin1(keys::kStopTime)).toLongLong();
        m_events->addItem(u"■ %1 — %2 — %3"_s.arg(
            activity.toString(), success ? u"SUCCESS"_s : u"FAILED"_s, shortTime(stopTime)
        ));
        m_events->scrollToBottom();
        m_previousActivity.clear();
    }

    ObserverProxy m_manager;
    QLabel* m_label;
    QListWidget* m_events;
    QVariantMap m_previousActivity;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    StatusWindow window;
    window.show();
    return app.exec();
}
