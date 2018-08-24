#-------------------------------------------------
#
# Project created by QtCreator 2018-07-17T22:06:44
#
#-------------------------------------------------

QT       += testlib network xml xmlpatterns qml sql

TARGET = tst_upnp_servicecontentdirectorytest
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

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/UpnpLibrary
LIBS += -L$$(MYLIBRARY)/$$QT_VERSION -l$$qtLibraryTarget(UpnpLibrary)

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/QmlApplication
LIBS += -L$$(MYLIBRARY)/$$QT_VERSION -l$$qtLibraryTarget(QmlApplication)

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/Youtube
LIBS += -L$$(MYLIBRARY)/$$QT_VERSION -l$$qtLibraryTarget(youtube)

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/multimedia
LIBS += -L$$(MYLIBRARY)/$$QT_VERSION -l$$qtLibraryTarget(mediadevice)

INCLUDEPATH += /opt/local/include
LIBS += -L/opt/local/lib -lavcodec -lavformat -lavutil -lswscale -lswresample

SOURCES += \
        tst_upnp_servicecontentdirectorytest.cpp
