#include <QApplication>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDateTime>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariantMap>
#include <QWidget>

#include "DBusConstants.h"
#include "managerproxy.h"

class StatusWindow : public QWidget {
  public:
    StatusWindow()
        : m_manager(wowcapd::dbus::kServiceName, wowcapd::dbus::kObjectPath,
                    QDBusConnection::sessionBus()) {
        setWindowTitle(QStringLiteral("wowcapd"));
        resize(480, 360);

        auto *layout = new QVBoxLayout(this);
        m_label = new QLabel(QStringLiteral("connecting..."), this);
        layout->addWidget(m_label);

        m_events = new QListWidget(this);
        layout->addWidget(m_events);

        connect(&m_manager, &ManagerProxy::EncounterDetected, this,
                &StatusWindow::onEncounterDetected);
        connect(&m_manager, &ManagerProxy::EncounterEnded, this, &StatusWindow::onEncounterEnded);
        connect(&m_manager, &ManagerProxy::DungeonDetected, this, &StatusWindow::onDungeonDetected);
        connect(&m_manager, &ManagerProxy::DungeonEnded, this, &StatusWindow::onDungeonEnded);

        auto *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &StatusWindow::refresh);
        timer->start(1000);
        refresh();
    }

  private:
    void refresh() {
        if (!m_manager.isValid()) {
            m_label->setText(QStringLiteral("wowcapd not reachable"));
            return;
        }
        QDBusReply<QVariantMap> reply = m_manager.Status();
        if (!reply.isValid()) {
            m_label->setText(QStringLiteral("Status() failed: %1").arg(reply.error().message()));
            return;
        }
        const QVariantMap status = reply.value();
        m_label->setText(QStringLiteral("state: %1\nwowActive: %2\nactiveCapture: %3")
                             .arg(status.value(QStringLiteral("state")).toString())
                             .arg(status.value(QStringLiteral("wowActive")).toBool())
                             .arg(status.value(QStringLiteral("activeCapture")).toString()));
    }

    static QString shortTime(const QString &isoTime) {
        return QDateTime::fromString(isoTime, Qt::ISODateWithMs)
            .toString(QStringLiteral("HH:mm:ss"));
    }

    void onEncounterDetected(int /*encounterId*/, const QString &encounterName,
                             const QString &difficulty, const QString &startTime) {
        m_events->addItem(
            QStringLiteral("▶ %1 (%2) — %3").arg(encounterName, difficulty, shortTime(startTime)));
        m_events->scrollToBottom();
    }

    void onEncounterEnded(int /*encounterId*/, const QString &encounterName, bool success,
                          const QString &stopTime) {
        m_events->addItem(QStringLiteral("■ %1 — %2 — %3")
                              .arg(encounterName,
                                   success ? QStringLiteral("KILL") : QStringLiteral("WIPE"),
                                   shortTime(stopTime)));
        m_events->scrollToBottom();
    }

    void onDungeonDetected(int /*zoneId*/, int mapId, int keystoneLevel, const QString &startTime) {
        m_events->addItem(QStringLiteral("▶ Mythic+ %1 (map %2) — %3")
                              .arg(keystoneLevel)
                              .arg(mapId)
                              .arg(shortTime(startTime)));
        m_events->scrollToBottom();
    }

    void onDungeonEnded(int mapId, int keystoneLevel, bool success, int durationMs,
                        const QString &stopTime) {
        m_events->addItem(QStringLiteral("■ Mythic+ %1 (map %2) — %3 (%4ms) — %5")
                              .arg(keystoneLevel)
                              .arg(mapId)
                              .arg(success ? QStringLiteral("TIMED") : QStringLiteral("DEPLETED"))
                              .arg(durationMs)
                              .arg(shortTime(stopTime)));
        m_events->scrollToBottom();
    }

    ManagerProxy m_manager;
    QLabel *m_label;
    QListWidget *m_events;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    StatusWindow window;
    window.show();
    return app.exec();
}
