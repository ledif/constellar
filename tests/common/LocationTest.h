#pragma once

#include <QObject>

class LocationTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void emptyLocationIsEmpty();
    void uiMapOnlyRoundTrips();
    void uiMapWithBoundsRoundTrips();
    void uiMapWithInvalidBoundsOmitsBoundsKey();
    void zoneOnlyRoundTrips();
    void bothHalvesRoundTrip();
    void displayNamePrefersZoneOverUiMap();
    void displayNameFallsBackToUiMapWhenZoneNameEmpty();
    void displayNameEmptyWhenNeitherSet();
    void toStringIsNoneWhenEmpty();
};
