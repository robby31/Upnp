#-------------------------------------------------
#
# Project created by QtCreator 2018-07-14T10:46:02
#
#-------------------------------------------------

QT       += testlib network xml xmlpatterns

QT       -= gui

TARGET = tst_upnpserviceconnectionmanagertest
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += sdk_no_version_check

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$(MYLIBRARY)/include/UpnpLibrary
LIBS += -L$$(MYLIBRARY)/lib -l$$qtLibraryTarget(UpnpLibrary)

INCLUDEPATH += $$(MYLIBRARY)/include/QmlApplication
LIBS += -L$$(MYLIBRARY)/lib -l$$qtLibraryTarget(QmlApplication)

SOURCES += \
        tst_upnpserviceconnectionmanagertest.cpp
