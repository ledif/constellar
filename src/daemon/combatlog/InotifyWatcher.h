#pragma once

#include <vector>

#include <QObject>
#include <QString>

class QSocketNotifier;

namespace constellar::inotify
{

struct WatchEvent
{
    bool created = false;
    bool deleted = false;
    bool modified = false;
    QString name;
};

class InotifyWatcher : public QObject
{
    Q_OBJECT

  public:
    explicit InotifyWatcher(QObject* parent = nullptr);
    ~InotifyWatcher() override;

    bool start(QString const& directory);
    void stop();
    bool isRunning() const;

  Q_SIGNALS:
    void events(std::vector<WatchEvent> const& events);

  private Q_SLOTS:
    void onReadyRead();

  private:
    int m_fd = -1;
    QSocketNotifier* m_notifier = nullptr;
};

}  // namespace constellar::inotify
