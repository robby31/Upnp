import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import MyComponents 1.0

Page {
    id: page
    width: 200
    height: 100

    property var upnpControlPoint

    Loader {
        id: loader

        anchors.fill: parent
    }

    function setRootDeviceListView() {
        loader.setSource("RootDeviceListView.qml", { model: upnpControlPoint.rootDevices })
    }

    function setRootDeviceDetails(index, iconurl) {
        loader.setSource("RootDeviceDetails.qml", { device: upnpControlPoint.rootDevices.at(index), iconurl: iconurl})
    }

    Component.onCompleted: {
        setRootDeviceListView()
    }
}
