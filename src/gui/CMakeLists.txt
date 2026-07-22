set(BUILD_SHARED_LIBS OFF)

# qml/ is a subdirectory of the module root so we need this for some reason
qt6_policy(SET QTP0004 NEW)

ecm_add_qml_module(constellar-qml
  URI "dev.ulduar.Constellar"
  VERSION 1.0
  GENERATE_PLUGIN_SOURCE
  SOURCES
    ObserverController.cpp
    ObserverController.h
    ActivityModel.cpp
    ActivityModel.h
  QML_FILES
    qml/Main.qml
    qml/StatusPage.qml
)

target_include_directories(constellar-qml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(constellar-qml
  PUBLIC
    Qt6::Core
    Qt6::Quick
    KF6::CoreAddons
    constellar::client
    constellar::common
  PRIVATE
    constellar::contract
)

add_executable(constellar main.cpp)

target_link_libraries(constellar
  PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickControls2
    KF6::Kirigami
    KF6::I18n
    KF6::CoreAddons
    KF6::IconThemes
    constellar-qml
)

install(TARGETS constellar)
