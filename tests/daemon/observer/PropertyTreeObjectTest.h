#pragma once

#include <QObject>

class PropertyTreeObjectTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void propertiesChangedMessageHasExpectedShape();
};
