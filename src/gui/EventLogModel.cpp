#include "EventLogModel.h"

EventLogModel::EventLogModel(QObject* parent) : QAbstractListModel(parent) {}

int EventLogModel::rowCount(QModelIndex const& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(m_entries.size());
}

QVariant EventLogModel::data(QModelIndex const& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    Entry const& entry = m_entries.at(index.row());
    switch (role)
    {
        case TextRole:
            return entry.text;
        case SuccessRole:
            return entry.success;
        case TimestampRole:
            return entry.epochMs;
        default:
            return {};
    }
}

QHash<int, QByteArray> EventLogModel::roleNames() const
{
    return {
        {TextRole, "message"},
        {SuccessRole, "success"},
        {TimestampRole, "timestamp"},
    };
}

void EventLogModel::addEntry(QString const& text, bool success, qint64 epochMs)
{
    int const row = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), row, row);
    m_entries.append({text, success, epochMs});
    endInsertRows();
}
