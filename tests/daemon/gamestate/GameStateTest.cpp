#include "GameStateTest.h"

#include <QSignalSpy>
#include <QTest>
#include <QVariantMap>

#include "ActivityKeys.h"
#include "GameState.h"
#include "Location.h"

namespace keys = constellar::keys;

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

void GameStateTest::setUiMapEmitsOnChange()
{
    GameState state;
    QSignalSpy spy(&state, &GameState::locationChanged);

    UiMap const uiMap{2393, QStringLiteral("Silvermoon City")};
    state.setUiMap(uiMap);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(
        state.location().value(QString::fromLatin1(keys::kZoneName)).toString(),
        QStringLiteral("Silvermoon City")
    );
    QCOMPARE(state.location().value(QString::fromLatin1(keys::kUiMapId)).toUInt(), 2393u);
}

void GameStateTest::setUiMapSkipsOnIdenticalValue()
{
    GameState state;
    UiMap const uiMap{2393, QStringLiteral("Silvermoon City")};
    state.setUiMap(uiMap);

    QSignalSpy spy(&state, &GameState::locationChanged);
    state.setUiMap(uiMap);

    QCOMPARE(spy.count(), 0);
}

void GameStateTest::setZoneEmitsOnChange()
{
    GameState state;
    QSignalSpy spy(&state, &GameState::locationChanged);

    Zone const zone{QStringLiteral("Sanctum of Light"), 0};
    state.setZone(zone);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(
        state.location().value(QString::fromLatin1(keys::kZoneName)).toString(),
        QStringLiteral("Sanctum of Light")
    );
}

void GameStateTest::setZoneSkipsOnIdenticalValue()
{
    GameState state;
    Zone const zone{QStringLiteral("Sanctum of Light"), 0};
    state.setZone(zone);

    QSignalSpy spy(&state, &GameState::locationChanged);
    state.setZone(zone);

    QCOMPARE(spy.count(), 0);
}

void GameStateTest::setUiMapThenSetZoneMergesBothHalves()
{
    GameState state;
    state.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City")});
    state.setZone(Zone{QStringLiteral("Sanctum of Light"), 0});

    QVariantMap const location = state.location();
    // Both halves merged: the resolved name comes from the zone, the id from the uiMap.
    QCOMPARE(
        location.value(QString::fromLatin1(keys::kZoneName)).toString(),
        QStringLiteral("Sanctum of Light")
    );
    QCOMPARE(location.value(QString::fromLatin1(keys::kUiMapId)).toUInt(), 2393u);
}

void GameStateTest::oneHalfUpdateDoesNotClearTheOther()
{
    GameState state;
    state.setUiMap(UiMap{2393, QStringLiteral("Silvermoon City")});
    state.setZone(Zone{QStringLiteral("Sanctum of Light"), 0});

    // Zoning back out fires ZONE_CHANGE again without a MAP_CHANGE — the uiMap half
    // must survive.
    state.setZone(Zone{QStringLiteral("Silvermoon City"), 0});

    QCOMPARE(state.location().value(QString::fromLatin1(keys::kUiMapId)).toUInt(), 2393u);
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
