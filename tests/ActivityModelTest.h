#pragma once

#include <QObject>

class ActivityModelTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void roleNamesExposeAllRoles();
    void beginActivityInsertsInProgressRow();
    void endActivityUpdatesMatchingInProgressRow();
    void beginActivityDeduplicatesByStartTime();
    void endActivityWithoutMatchPrependsRow();
};
