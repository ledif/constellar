#include "GameStateTest.h"

#include <QSignalSpy>
#include <QTest>
#include <QVariantMap>

#include "GameState.h"

void GameStateTest::setActivityEmitsOnChange() {
    GameState state;
    QSignalSpy spy(&state, &GameState::activityChanged);

    const QVariantMap activity{{"type", "encounter"}};
    state.setActivity(activity);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(state.activity(), activity);
}

void GameStateTest::setActivitySkipsOnIdenticalBag() {
    GameState state;
    const QVariantMap activity{{"type", "encounter"}};
    state.setActivity(activity);

    QSignalSpy spy(&state, &GameState::activityChanged);
    state.setActivity(activity);

    QCOMPARE(spy.count(), 0);
}

void GameStateTest::setZoneEmitsOnChange() {
    GameState state;
    QSignalSpy spy(&state, &GameState::zoneChanged);

    const QVariantMap zone{{"zoneName", "Dornogal"}};
    state.setZone(zone);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(state.zone(), zone);
}

void GameStateTest::setZoneSkipsOnIdenticalBag() {
    GameState state;
    const QVariantMap zone{{"zoneName", "Dornogal"}};
    state.setZone(zone);

    QSignalSpy spy(&state, &GameState::zoneChanged);
    state.setZone(zone);

    QCOMPARE(spy.count(), 0);
}

void GameStateTest::clearActivityEmptiesActivity() {
    GameState state;
    state.setActivity(QVariantMap{{"type", "encounter"}});
    QSignalSpy spy(&state, &GameState::activityChanged);

    state.clearActivity();

    QVERIFY(state.activity().isEmpty());
    QCOMPARE(spy.count(), 1);
}

void GameStateTest::endActivityEmitsThenClears() {
    GameState state;
    const QVariantMap activity{{"type", "encounter"}};
    state.setActivity(activity);

    QSignalSpy endedSpy(&state, &GameState::activityEnded);
    QSignalSpy changedSpy(&state, &GameState::activityChanged);

    const QVariantMap ended{{"type", "encounter"}, {"success", true}};
    state.endActivity(ended);

    QCOMPARE(endedSpy.count(), 1);
    QCOMPARE(endedSpy.at(0).at(0).toMap(), ended);
    QCOMPARE(changedSpy.count(), 1);
    QVERIFY(state.activity().isEmpty());
}

QTEST_MAIN(GameStateTest)
