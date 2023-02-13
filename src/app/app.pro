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
    bagrov.h \
    calculation.h \
    config.h \
    constants.h \
    dbaseField.h \
    dbaseReader.h \
    dbaseWriter.h \
    effectivenessunsealed.h \
    helpers.h \
    initvalues.h \
    main.h \
    mainwindow.h \
    pdr.h \
    saxhandler.h

SOURCES += \
    bagrov.cpp \
    calculation.cpp \
    config.cpp \
    dbaseField.cpp \
    dbaseReader.cpp \
    dbaseWriter.cpp \
    effectivenessunsealed.cpp \
    helpers.cpp \
    initvalues.cpp \
    main.cpp \
    mainwindow.cpp \
    pdr.cpp \
    saxhandler.cpp

#RC_FILE += AbimoQt.rc
#OTHER_FILES += release/config.xml
#INSTALLS += TARGET
