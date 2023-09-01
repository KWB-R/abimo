# Load common settings
! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

QMAKE_CXXFLAGS += -std=c++11

#TEMPLATE = app
TARGET = Abimo
#DEPENDPATH += .
#INCLUDEPATH += .
QT += \
    core \
    widgets \
    xml \

#CONFIG += console

# https://stackoverflow.com/a/4563512
CONFIG *= debug_and_release debug_and_release_target

HEADERS += \
    abimoInputRecord.h \
    abimoOutputRecord.h \
    abimoReader.h \
    abimoWriter.h \
    bagrov.h \
    calculation.h \
    constants.h \
    counters.h \
    dbaseField.h \
    dbaseFile.h \
    dbaseReader.h \
    dbaseWriter.h \
    effectivenessUnsealed.h \
    helpers.h \
    initValues.h \
    intermediateResults.h \
    main.h \
    mainWindow.h \
    saxHandler.h \
    soilAndVegetation.h \
    structs.h \
    usageConfiguration.h

SOURCES += \
    abimoInputRecord.cpp \
    abimoOutputRecord.cpp \
    abimoReader.cpp \
    abimoWriter.cpp \
    bagrov.cpp \
    calculation.cpp \
    counters.cpp \
    dbaseField.cpp \
    dbaseFile.cpp \
    dbaseReader.cpp \
    dbaseWriter.cpp \
    effectivenessUnsealed.cpp \
    helpers.cpp \
    initValues.cpp \
    intermediateResults.cpp \
    main.cpp \
    mainWindow.cpp \
    saxHandler.cpp \
    soilAndVegetation.cpp \
    usageConfiguration.cpp

#RC_FILE += AbimoQt.rc
#OTHER_FILES += release/config.xml
#INSTALLS += TARGET
