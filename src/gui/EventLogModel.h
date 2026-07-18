#pragma once

#include <qqmlintegration.h>
#include <QAbstractListModel>
#include <QList>
#include <QString>

class EventLogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by ObserverController")

  public:
    enum Role
    {
        TextRole = Qt::UserRole + 1,
        SuccessRole,
        TimestampRole,
    };

    Q_ENUM(Role)

    explicit EventLogModel(QObject* parent = nullptr);

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    QVariant data(QModelIndex const& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addEntry(QString const& text, bool success, qint64 epochMs);

  private:
    struct Entry
    {
        QString text;
        bool success = false;
        qint64 epochMs = 0;
    };

    QList<Entry> m_entries;
};
