#pragma once

#include <QObject>
#include <QVariantMap>

// A property-bag activity object on the bus — the node the .Activity family of
// adaptors attach to. Mounted today at /activity/current; TASK-010 reuses it for
// historical /activities/<id> objects.
class ActivityObject : public QObject
{
    Q_OBJECT

  public:
    explicit ActivityObject(QVariantMap bag, QObject* parent = nullptr);

    QVariantMap const& bag() const
    {
        return m_bag;
    }

  private:
    QVariantMap m_bag;
};
