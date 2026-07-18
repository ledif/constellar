#include "GuiQmlLoadTest.h"

#include <QQmlApplicationEngine>
#include <QSignalSpy>
#include <QTest>

void GuiQmlLoadTest::loadsWithoutErrors()
{
    QQmlApplicationEngine engine;
    QSignalSpy warnings(&engine, &QQmlApplicationEngine::warnings);

    engine.loadFromModule("io.github.ledif.constellar", "Main");

    QVERIFY2(
        !engine.rootObjects().isEmpty(), "Main.qml failed to instantiate; see stderr for QML errors"
    );
    QVERIFY2(warnings.isEmpty(), "QML engine emitted warnings while loading Main.qml");
}

QTEST_MAIN(GuiQmlLoadTest)
