#include <QApplication>
#include <QDBusConnection>
#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QWidget>

#include "ActivityKeys.h"
#include "DBusConstants.h"
#include "observerproxy.h"

namespace keys = constellar::keys;

// Still a stub label (ADR-012) -- polls Activity/Zone on a timer rather than
// wiring up org.freedesktop.DBus.Properties.PropertiesChanged, since nothing
// here needs sub-second latency yet.
class StatusWindow : public QWidget {
  public:
    StatusWindow()
        : m_manager(constellar::dbus::kServiceName, constellar::dbus::kObjectPath,
                    QDBusConnection::sessionBus()) {
        setWindowTitle(QStringLiteral("Constellar"));
        resize(480, 360);

        auto *layout = new QVBoxLayout(this);
        m_label = new QLabel(QStringLiteral("connecting..."), this);
        layout->addWidget(m_label);

        m_events = new QListWidget(this);
        layout->addWidget(m_events);

        connect(&m_manager, &ObserverProxy::ActivityEnded, this, &StatusWindow::onActivityEnded);

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &StatusWindow::refresh);
        timer->start(1000);
        refresh();
    }

  private:
    void refresh() {
        if (!m_manager.isValid()) {
            m_label->setText(QStringLiteral("constellard not reachable"));
            return;
        }

        const QVariantMap activity = m_manager.property("Activity").toMap();
        const QVariantMap zone = m_manager.property("Zone").toMap();

        if (m_previousActivity.isEmpty() && !activity.isEmpty()) {
            m_events->addItem(QStringLiteral("▶ %1").arg(describeActivity(activity)));
            m_events->scrollToBottom();
        }
        m_previousActivity = activity;

        m_label->setText(
            QStringLiteral("activity: %1\nzone: %2")
                .arg(activity.isEmpty() ? QStringLiteral("none") : describeActivity(activity),
                     zone.value(QString::fromLatin1(keys::kZoneName)).toString()));
    }

    static QString shortTime(qint64 epochMs) {
        return QDateTime::fromMSecsSinceEpoch(epochMs).toString(QStringLiteral("HH:mm:ss"));
    }

    static QString describeActivity(const QVariantMap &activity) {
        const QString type = activity.value(QString::fromLatin1(keys::kType)).toString();
        if (type == QString::fromLatin1(keys::kTypeEncounter)) {
            return QStringLiteral("%1 %2").arg(
                activity.value(QString::fromLatin1(keys::kDifficulty)).toString(),
                activity.value(QString::fromLatin1(keys::kEncounterName)).toString());
        }
        if (type == QString::fromLatin1(keys::kTypeDungeon)) {
            return QStringLiteral("Mythic+ %1")
                .arg(activity.value(QString::fromLatin1(keys::kKeystoneLevel)).toString());
        }
        return QStringLiteral("unknown");
    }

    void onActivityEnded(const QVariantMap &activity) {
        const bool success = activity.value(QString::fromLatin1(keys::kSuccess)).toBool();
        const qint64 stopTime = activity.value(QString::fromLatin1(keys::kStopTime)).toLongLong();
        m_events->addItem(QStringLiteral("■ %1 — %2 — %3")
                              .arg(describeActivity(activity),
                                   success ? QStringLiteral("SUCCESS") : QStringLiteral("FAILED"),
                                   shortTime(stopTime)));
        m_events->scrollToBottom();
        m_previousActivity.clear();
    }

    ObserverProxy m_manager;
    QLabel *m_label;
    QListWidget *m_events;
    QVariantMap m_previousActivity;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    StatusWindow window;
    window.show();
    return app.exec();
}
