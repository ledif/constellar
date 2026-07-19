#pragma once

#include <qqmlintegration.h>
#include <QAbstractListModel>
#include <QList>
#include <QString>

class ActivityModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by ObserverController")

  public:
    enum Role
    {
        TitleRole = Qt::UserRole + 1,
        StateRole,
        StartTimeRole,
        StopTimeRole,
        DurationRole,
    };
    Q_ENUM(Role)

    enum ActivityState
    {
        InProgress,
        Success,
        Failure,
    };
    Q_ENUM(ActivityState)

    explicit ActivityModel(QObject* parent = nullptr);

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    QVariant data(QModelIndex const& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void beginActivity(QString const& title, qint64 startTime);
    void endActivity(
        QString const& title, qint64 startTime, bool success, qint64 stopTime, qint64 durationMs
    );

  private:
    struct Entry
    {
        QString title;
        ActivityState state = InProgress;
        qint64 startTime = 0;
        qint64 stopTime = 0;
        qint64 durationMs = 0;
    };

    QList<Entry> m_entries;
};
