#include "GameStateTest.h"

#include <QSignalSpy>
#include <QTest>
#include <QVariantMap>

#include "GameState.h"

void GameStateTest::setActivityEmitsOnChange()
{
    GameState state;
    QSignalSpy spy(&state, &GameState::activityChanged);

    QVariantMap const activity{{"type", "encounter"}};
    state.setActivity(activity);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(state.activity(), activity);
}

void GameStateTest::setActivitySkipsOnIdenticalBag()
{
    GameState state;
    QVariantMap const activity{{"type", "encounter"}};
    state.setActivity(activity);

    QSignalSpy spy(&state, &GameState::activityChanged);
    state.setActivity(activity);

    QCOMPARE(spy.count(), 0);
}

void GameStateTest::setZoneEmitsOnChange()
{
    GameState state;
    QSignalSpy spy(&state, &GameState::zoneChanged);

    QVariantMap const zone{{"zoneName", "Dornogal"}};
    state.setZone(zone);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(state.zone(), zone);
}

void GameStateTest::setZoneSkipsOnIdenticalBag()
{
    GameState state;
    QVariantMap const zone{{"zoneName", "Dornogal"}};
    state.setZone(zone);

    QSignalSpy spy(&state, &GameState::zoneChanged);
    state.setZone(zone);

    QCOMPARE(spy.count(), 0);
}

void GameStateTest::endActivityEmitsThenClears()
{
    GameState state;
    QVariantMap const activity{{"type", "encounter"}};
    state.setActivity(activity);

    QSignalSpy endedSpy(&state, &GameState::activityEnded);
    QSignalSpy changedSpy(&state, &GameState::activityChanged);

    QVariantMap const ended{{"type", "encounter"}, {"success", true}};
    state.endActivity(ended);

    QCOMPARE(endedSpy.count(), 1);
    QCOMPARE(endedSpy.at(0).at(0).toMap(), ended);
    QCOMPARE(changedSpy.count(), 1);
    QVERIFY(state.activity().isEmpty());
}

QTEST_MAIN(GameStateTest)
