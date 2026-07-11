#include <QApplication>
#include <QDBusConnection>
#include <QDBusReply>
#include <QLabel>
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

        auto *layout = new QVBoxLayout(this);
        m_label = new QLabel(QStringLiteral("connecting..."), this);
        layout->addWidget(m_label);

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

    ManagerProxy m_manager;
    QLabel *m_label;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    StatusWindow window;
    window.show();
    return app.exec();
}
