#pragma once

#include <QObject>

class ActivityTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void emptyMapIsNone();
    void encounterMapsDifficultyAndName();
    void dungeonMapsKeystoneLevel();
    void unrecognizedTypeIsNone();
};
