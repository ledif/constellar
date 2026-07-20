#include "ActivityModelTest.h"

#include <QSignalSpy>
#include <QTest>

#include "ActivityModel.h"

void ActivityModelTest::roleNamesExposeAllRoles()
{
    ActivityModel model;
    QHash<int, QByteArray> const roles = model.roleNames();
    QList<QByteArray> const names = roles.values();

    QVERIFY(names.contains("title"));
    QVERIFY(names.contains("activityState"));
    QVERIFY(names.contains("startTime"));
    QVERIFY(names.contains("stopTime"));
    QVERIFY(names.contains("durationMs"));
}

void ActivityModelTest::beginActivityInsertsInProgressRow()
{
    ActivityModel model;
    model.beginActivity(QStringLiteral("Mythic Midnight Falls"), 1000);

    QCOMPARE(model.rowCount(), 1);
    QModelIndex const idx = model.index(0);
    QCOMPARE(
        model.data(idx, ActivityModel::TitleRole).toString(),
        QStringLiteral("Mythic Midnight Falls")
    );
    QCOMPARE(model.data(idx, ActivityModel::StateRole).toInt(), int(ActivityModel::InProgress));
}

void ActivityModelTest::endActivityUpdatesMatchingInProgressRow()
{
    ActivityModel model;
    model.beginActivity(QStringLiteral("Mythic Midnight Falls"), 1000);
    QSignalSpy dataChangedSpy(&model, &QAbstractItemModel::dataChanged);

    model.endActivity(QStringLiteral("Mythic Midnight Falls"), 1000, true, 2000, 1000);

    QCOMPARE(model.rowCount(), 1);
    QModelIndex const idx = model.index(0);
    QCOMPARE(model.data(idx, ActivityModel::StateRole).toInt(), int(ActivityModel::Success));
    QCOMPARE(model.data(idx, ActivityModel::StopTimeRole).toLongLong(), 2000);
    QCOMPARE(model.data(idx, ActivityModel::DurationRole).toLongLong(), 1000);
    QCOMPARE(dataChangedSpy.count(), 1);
}

void ActivityModelTest::beginActivityDeduplicatesByStartTime()
{
    ActivityModel model;
    model.beginActivity(QStringLiteral("Mythic Midnight Falls"), 1000);
    model.beginActivity(QStringLiteral("Mythic Midnight Falls"), 1000);

    QCOMPARE(model.rowCount(), 1);
}

void ActivityModelTest::endActivityWithoutMatchPrependsRow()
{
    ActivityModel model;
    model.endActivity(QStringLiteral("Mythic+ Key +10"), 1000, false, 2000, 1000);

    QCOMPARE(model.rowCount(), 1);
    QModelIndex const idx = model.index(0);
    QCOMPARE(model.data(idx, ActivityModel::StateRole).toInt(), int(ActivityModel::Failure));
}

QTEST_MAIN(ActivityModelTest)
