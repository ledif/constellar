#pragma once

#include <QObject>

class LocationTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void emptyLocationIsEmpty();
    void uiMapEmitsWireKeys();
    void zoneEmitsCategory();
    void zoneCategoryMappings();
    void locationRoundTripsThroughWire();
    void displayNamePrefersZoneOverUiMap();
    void displayNameFallsBackToUiMapWhenZoneNameEmpty();
    void displayNameEmptyWhenNeitherSet();
    void toStringIsNoneWhenEmpty();
};
