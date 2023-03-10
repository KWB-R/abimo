# Load common settings
! include( ../common.pri ) {
    error( "Couldn't find the common.pri file!" )
}

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
    abimoReader.h \
    abimoRecord.h \
    bagrov.h \
    calculation.h \
    config.h \
    constants.h \
    counters.h \
    dbaseField.h \
    dbaseFile.h \
    dbaseReader.h \
    dbaseWriter.h \
    effectivenessUnsealed.h \
    helpers.h \
    initValues.h \
    main.h \
    mainWindow.h \
    pdr.h \
    saxHandler.h

SOURCES += \
    abimoReader.cpp \
    abimoRecord.cpp \
    bagrov.cpp \
    calculation.cpp \
    config.cpp \
    counters.cpp \
    dbaseField.cpp \
    dbaseFile.cpp \
    dbaseReader.cpp \
    dbaseWriter.cpp \
    effectivenessUnsealed.cpp \
    helpers.cpp \
    initValues.cpp \
    main.cpp \
    mainWindow.cpp \
    pdr.cpp \
    saxHandler.cpp

#RC_FILE += AbimoQt.rc
#OTHER_FILES += release/config.xml
#INSTALLS += TARGET
