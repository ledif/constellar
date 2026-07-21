#pragma once

#include <QObject>

class ActivityMetadataTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void encounterMetadataMapsDifficultyAndName();
    void encounterMetadataUnknownDifficultyIdIsUnknown();
    void encounterEndedMetadataAddsSuccessAndStopTime();
    void dungeonMetadataMapsKeystoneLevel();
    void dungeonEndedMetadataAddsSuccessDurationAndStopTime();
};
