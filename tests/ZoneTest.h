#pragma once

#include <QObject>

class ZoneTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void emptyMapIsEmpty();
    void mapsNameAndMapId();
};
