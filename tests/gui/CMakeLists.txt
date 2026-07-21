constellar_add_test(gui_qml_load_test
    SOURCES GuiQmlLoadTest.h GuiQmlLoadTest.cpp
    LIBS Qt6::Gui Qt6::Qml Qt6::Quick constellar-qml
)

set_tests_properties(gui_qml_load_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

constellar_add_test(activitymodel_test
    SOURCES ActivityModelTest.h ActivityModelTest.cpp
    LIBS Qt6::Gui Qt6::Qml Qt6::Quick constellar-qml
)

set_tests_properties(activitymodel_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
