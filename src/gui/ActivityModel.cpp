#include "ActivityModel.h"

ActivityModel::ActivityModel(QObject* parent) : QAbstractListModel(parent) {}

int ActivityModel::rowCount(QModelIndex const& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_entries.size());
}

QVariant ActivityModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    Entry const& entry = m_entries.at(index.row());
    switch (role)
    {
        case TitleRole:
            return entry.title;
        case StateRole:
            return entry.state;
        case StartTimeRole:
            return entry.startTime;
        case StopTimeRole:
            return entry.stopTime;
        case DurationRole:
            return entry.durationMs;
        default:
            return {};
    }
}

QHash<int, QByteArray> ActivityModel::roleNames() const
{
    return {
        {TitleRole, "title"},       {StateRole, "activityState"}, {StartTimeRole, "startTime"},
        {StopTimeRole, "stopTime"}, {DurationRole, "durationMs"},
    };
}

void ActivityModel::beginActivity(QString const& title, qint64 startTime)
{
    if (startTime != 0)
    {
        for (Entry const& entry : m_entries)
            if (entry.startTime == startTime)
                return;
    }

    beginInsertRows(QModelIndex(), 0, 0);
    m_entries.prepend({title, InProgress, startTime, 0, 0});
    endInsertRows();
}

void ActivityModel::endActivity(
    QString const& title, qint64 startTime, bool success, qint64 stopTime, qint64 durationMs
)
{
    int targetRow = -1;

    if (startTime != 0)
    {
        for (int i = 0; i < m_entries.size(); ++i)
        {
            if (m_entries.at(i).startTime == startTime)
            {
                targetRow = i;
                break;
            }
        }
    }

    if (targetRow < 0)
    {
        for (int i = 0; i < m_entries.size(); ++i)
        {
            if (m_entries.at(i).state == InProgress)
            {
                targetRow = i;
                break;
            }
        }
    }

    if (targetRow < 0)
    {
        beginInsertRows(QModelIndex(), 0, 0);
        m_entries.prepend({title, success ? Success : Failure, startTime, stopTime, durationMs});
        endInsertRows();
        return;
    }

    Entry& entry = m_entries[targetRow];
    entry.state = success ? Success : Failure;
    entry.stopTime = stopTime;
    entry.durationMs = durationMs;

    QModelIndex const idx = index(targetRow);
    Q_EMIT dataChanged(idx, idx, {StateRole, StopTimeRole, DurationRole});
}
