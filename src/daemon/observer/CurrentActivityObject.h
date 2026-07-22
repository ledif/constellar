#pragma once

#include <QObject>
#include <QVariantMap>

// The `/activity/current` object (RFC-006): a second, typed-interface projection of
// the same in-progress-activity bag GameState already holds. Created fresh when an
// activity starts and destroyed when it ends, so its composed adaptors' interface
// set never has to change shape mid-lifetime (TASK-009).
class CurrentActivityObject : public QObject
{
    Q_OBJECT

  public:
    explicit CurrentActivityObject(QVariantMap bag, QObject* parent = nullptr);

    QVariantMap const& bag() const
    {
        return m_bag;
    }

  private:
    QVariantMap m_bag;
};
