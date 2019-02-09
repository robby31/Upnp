#-------------------------------------------------
#
# Project created by QtCreator 2017-02-04T11:49:54
#
#-------------------------------------------------

QT       += network xml qml sql

TARGET = UpnpLibrary
TARGET = $$qtLibraryTarget($$TARGET)

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++14

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

!exists($$(MYLIBRARY)) {
    error("variable MYLIBRARY not set.")
}

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/QmlApplication

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/Streaming

INCLUDEPATH += $$(MYLIBRARY)/$$QT_VERSION/include/multimedia

INCLUDEPATH += /opt/local/include

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    upnpcontrolpoint.cpp \
    ssdpmessage.cpp \
    upnprootdevice.cpp \
    upnpservice.cpp \
    upnpdevice.cpp \
    upnpobject.cpp \
    upnptimer.cpp \
    upnperror.cpp \
    Http/httpserver.cpp \
    Http/httprequest.cpp \
    soapaction.cpp \
    soapactionresponse.cpp \
    didllite.cpp \
    Http/elapsedtimer.cpp \
    eventresponse.cpp \
    statevariableitem.cpp \
    Http/httprange.cpp \
    upnpservicedescription.cpp \
    upnpdevicedescription.cpp \
    xmldescription.cpp \
    upnprootdevicedescription.cpp \
    upnpdescription.cpp \
    Services/serviceconnectionmanager.cpp \
    Services/servicecontentdirectory.cpp \
    Services/abstractservice.cpp \
    xmlevent.cpp \
    dlna/dlnamusictrackfile.cpp \
    dlna/dlnaitem.cpp \
    dlna/dlnamusictrack.cpp \
    dlna/dlnavideoitem.cpp \
    dlna/dlnayoutubevideo.cpp \
    dlna/dlnavideofile.cpp \
    dlna/dlnaresource.cpp \
    dlna/dlnastoragefolder.cpp \
    dlna/dlnafolder.cpp \
    dlna/dlnarootfolder.cpp \
    dlna/cached/dlnacachedmusictrack.cpp \
    dlna/cached/dlnacachednetworkvideo.cpp \
    dlna/cached/dlnacachedvideo.cpp \
    dlna/cached/dlnacachedrootfolder.cpp \
    dlna/cached/dlnacachedgroupedfoldermetadata.cpp \
    dlna/cached/dlnacachedfoldermetadata.cpp \
    dlna/cached/dlnacachedfolder.cpp \
    dlna/cached/medialibrary.cpp \
    Devices/devicesmodel.cpp \
    Devices/mediarenderer.cpp \
    Devices/mediarenderermodel.cpp \
    upnpactionreply.cpp \
    dlna/dlnanetworkvideo.cpp

HEADERS += \
    upnpcontrolpoint.h \
    ssdpmessage.h \
    upnprootdevice.h \
    upnpservice.h \
    upnpdevice.h \
    upnpobject.h \
    upnptimer.h \
    upnperror.h \
    Http/httpserver.h \
    Http/httprequest.h \
    soapaction.h \
    soapactionresponse.h \
    didllite.h \
    Http/elapsedtimer.h \
    eventresponse.h \
    statevariableitem.h \
    Http/httprange.h \
    upnpservicedescription.h \
    upnpdevicedescription.h \
    xmldescription.h \
    upnprootdevicedescription.h \
    upnpdescription.h \
    Services/serviceconnectionmanager.h \
    Services/servicecontentdirectory.h \
    Services/abstractservice.h \
    xmlevent.h \
    dlna/dlnaitem.h \
    dlna/dlnamusictrackfile.h \
    dlna/dlnavideoitem.h \
    dlna/dlnamusictrack.h \
    dlna/dlnayoutubevideo.h \
    dlna/dlnavideofile.h \
    dlna/dlnaresource.h \
    dlna/dlnastoragefolder.h \
    dlna/dlnarootfolder.h \
    dlna/dlnafolder.h \
    dlna/cached/dlnacachedmusictrack.h \
    dlna/cached/dlnacachedrootfolder.h \
    dlna/cached/dlnacachednetworkvideo.h \
    dlna/cached/dlnacachedvideo.h \
    dlna/cached/dlnacachedgroupedfoldermetadata.h \
    dlna/cached/dlnacachedfoldermetadata.h \
    dlna/cached/dlnacachedfolder.h \
    dlna/cached/medialibrary.h \
    Devices/devicesmodel.h \
    Devices/mediarenderer.h \
    Devices/mediarenderermodel.h \
    upnpactionreply.h \
    dlna/dlnanetworkvideo.h

DISTFILES += \
    doc/AVTransport/UPnP-av-AVTransport-v1-Service-20020625.pdf \
    doc/AVTransport/UPnP-av-AVTransport-v1-Service-AnnexA-20101006.pdf \
    doc/AVTransport/UPnP-av-AVTransport-v2-Service-20080930.pdf \
    doc/AVTransport/UPnP-av-AVTransport-v2-Service-AnnexA-20101006 2.pdf \
    doc/AVTransport/UPnP-av-AVTransport-v2-Service-AnnexA-20101006.pdf \
    doc/AVTransport/UPnP-av-AVTransport-v3-Service-20130331.pdf \
    doc/ConnectionManager/UPnP-av-ConnectionManager-v1-Service-20020625.pdf \
    doc/ConnectionManager/UPnP-av-ConnectionManager-v1-Service-AnnexA-20101006.pdf \
    doc/ConnectionManager/UPnP-av-ConnectionManager-v2-Service-20060531.pdf \
    doc/ConnectionManager/UPnP-av-ConnectionManager-v2-Service-AnnexA-20101006.pdf \
    doc/ConnectionManager/UPnP-av-ConnectionManager-v3-Service-20130331.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v1-Service-20020625.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v1-Service-AnnexA-20101006.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v2-Service-20060531.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v2-Service-AnnexA-20101006.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v3-Service-20080930.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v3-Service-AnnexA-20101006.pdf \
    doc/ContentDirectory/UPnP-av-ContentDirectory-v4-Service-20150319.pdf \
    doc/DeviceArchitecture/UPnP-arch-DeviceArchitecture-v1.0-20081015.pdf \
    doc/DeviceArchitecture/UPnP-arch-DeviceArchitecture-v1.1-20081015.pdf \
    doc/DeviceArchitecture/UPnP-arch-DeviceArchitecture-v1.1-AnnexA-20110315.pdf \
    doc/DeviceArchitecture/UPnP-arch-DeviceArchitecture-v2.0-20150220.pdf \
    doc/MediaRenderer/UPnP-av-MediaRenderer-v1-Device-20020625.pdf \
    doc/MediaRenderer/UPnP-av-MediaRenderer-v2-Device-20080930.pdf \
    doc/MediaRenderer/UPnP-av-MediaRenderer-v3-Device-20130331.pdf \
    doc/MediaServer/UPnP-av-MediaServer-v1-Device-20020625.pdf \
    doc/MediaServer/UPnP-av-MediaServer-v2-Device-20060531.pdf \
    doc/MediaServer/UPnP-av-MediaServer-v3-Device-20080930.pdf \
    doc/MediaServer/UPnP-av-MediaServer-v4-Device-20130331.pdf \
    doc/RenderingControl/UPnP-av-RenderingControl-v1-Service-20020625.pdf \
    doc/RenderingControl/UPnP-av-RenderingControl-v1-Service-AnnexA-20101006.pdf \
    doc/RenderingControl/UPnP-av-RenderingControl-v2-Service-20060531.pdf \
    doc/RenderingControl/UPnP-av-RenderingControl-v2-Service-AnnexA-20101006.pdf \
    doc/RenderingControl/UPnP-av-RenderingControl-v3-Service-20130331.pdf \
    doc/AVTransport/AVTransport1.SyntaxTests.xml \
    doc/AVTransport/AVTransport2.SyntaxTests.xml \
    doc/ConnectionManager/ConnectionManager1.SyntaxTests.xml \
    doc/ConnectionManager/ConnectionManager1.xml \
    doc/ConnectionManager/ConnectionManager2.SyntaxTests.xml \
    doc/ConnectionManager/ConnectionManager2.xml \
    doc/ContentDirectory/ContentDirectory1.SyntaxTests.xml \
    doc/ContentDirectory/ContentDirectory1.xml \
    doc/ContentDirectory/ContentDirectory2.SyntaxTests.xml \
    doc/ContentDirectory/ContentDirectory2.xml \
    doc/ContentDirectory/ContentDirectory3.SyntaxTests.xml \
    doc/ContentDirectory/ContentDirectory3.xml \
    doc/MediaRenderer/MediaRenderer1.xml \
    doc/MediaRenderer/MediaRenderer2.xml \
    doc/MediaServer/MediaServer1.xml \
    doc/MediaServer/MediaServer2.xml \
    doc/MediaServer/MediaServer3.xml \
    doc/RenderingControl/RenderingControl1.SyntaxTests.xml \
    doc/RenderingControl/RenderingControl1.xml \
    doc/RenderingControl/RenderingControl2.SyntaxTests.xml \
    doc/RenderingControl/RenderingControl2.xml \
    xml schema/upnp_device.xsd \
    xml schema/upnp_service.xsd \
    xml schema/upnp_control.xsd \
    xml schema/upnp_error.xsd \
    xml schema/upnp_event.xsd \
    xml schema/upnp_control_service_connection_manager.xsd \
    xml schema/upnp_service_connection_manager.xsd \
    xml schema/didl-Lite.xsd

installPath = $$(MYLIBRARY)/$$QT_VERSION
target.path = $$installPath
INSTALLS += target

installIncludePath = $$installPath/include/UpnpLibrary

h_include.files = *.h
h_include.path = $$installIncludePath
INSTALLS += h_include

http.files = Http/*.h
http.path = $$installIncludePath/Http
INSTALLS += http

services.files = Services/*.h
services.path = $$installIncludePath/Services
INSTALLS += services

devices.files = Devices/*.h
devices.path = $$installIncludePath/Devices
INSTALLS += devices

dlna.files = Dlna/*.h
dlna.path = $$installIncludePath/dlna
INSTALLS += dlna

dlna_cached.files = Dlna/Cached/*.h
dlna_cached.path = $$installIncludePath/dlna/cached
INSTALLS += dlna_cached

RESOURCES += \
    schema.qrc
