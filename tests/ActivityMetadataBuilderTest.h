#pragma once

#include <QObject>

class ActivityMetadataBuilderTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void encounterMetadataMapsDifficultyAndName();
    void encounterMetadataUnknownDifficultyIdIsUnknown();
    void encounterEndedMetadataAddsSuccessAndStopTime();
    void dungeonMetadataMapsKeystoneLevel();
    void dungeonEndedMetadataAddsSuccessDurationAndStopTime();
    void zoneMetadataMapsMapIdAndZoneName();
};
