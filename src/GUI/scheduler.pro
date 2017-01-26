QT += qml quick

CONFIG += c++14 qml_debug

SOURCES += main.cpp \
    ../Model_Engine/BuildScanPair.cpp \
    ../Model_Engine/CommandProcessor.cpp \
    ../Model_Engine/Scheduler.cpp \
    ../Model_Engine/Timeline.cpp \
    ../Model_Engine/Utility.cpp

RESOURCES += \
    qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    QueryRecords.h \
    CommandRecords.h \
    ../Model_Engine/BuildScanPair.h \
    ../Model_Engine/CommandProcessor.h \
    ../Model_Engine/ControlCommand.h \
    ../Model_Engine/EventQueue.h \
    ../Model_Engine/ModelEngine.h \
    ../Model_Engine/ModelState.h \
    ../Model_Engine/Query.h \
    ../Model_Engine/ScanPair.h \
    ../Model_Engine/Scheduler.h \
    ../Model_Engine/Settings.h \
    ../Model_Engine/Timeline.h \
    ../Model_Engine/Utility.h \
    QMLSchedulerController.h \
    TimelineGraph.h \
    ../Model_Engine/ModelException.h
