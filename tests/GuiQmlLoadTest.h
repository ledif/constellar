#pragma once

#include <QObject>

class GuiQmlLoadTest : public QObject
{
    Q_OBJECT

  private Q_SLOTS:
    void loadsWithoutErrors();
};
