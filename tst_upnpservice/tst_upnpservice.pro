#-------------------------------------------------
#
# Project created by QtCreator 2018-05-19T16:53:39
#
#-------------------------------------------------

QT       += testlib network xml sql qml

TARGET = tst_upnpservicetest
CONFIG   += console
CONFIG   -= app_bundle

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

INCLUDEPATH += $$(MYLIBRARY)/include/analyzer
LIBS += -L$$(MYLIBRARY)/lib -l$$qtLibraryTarget(analyzer)

INCLUDEPATH += $$(MYLIBRARY)/include/QmlApplication
LIBS += -L$$(MYLIBRARY)/lib -l$$qtLibraryTarget(QmlApplication)

INCLUDEPATH += $$(MYLIBRARY)/include/multimedia
LIBS += -L$$(MYLIBRARY)/lib -l$$qtLibraryTarget(mediadevice)

INCLUDEPATH += $$(MYLIBRARY)/include/Streaming
LIBS += -L$$(MYLIBRARY)/lib -l$$qtLibraryTarget(streaming)

INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib -lavcodec -lavformat -lavutil -lswscale -lswresample

SOURCES += \
        tst_upnpservicetest.cpp
