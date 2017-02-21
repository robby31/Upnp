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
        loader.setSource("RootDeviceListView.qml", { model: upnpControlPoint.remoteRootDevices })
    }

    function setRootDeviceDetails(index, iconurl) {
        loader.setSource("RootDeviceDetails.qml", { device: upnpControlPoint.remoteRootDevices.at(index), rootIndex: index, iconurl: iconurl})
    }

    function setServiceDetails(service, rootIndex, rootIconUrl) {
        loader.setSource("ServiceDetails.qml", { service: service, rootIndex: rootIndex, rootIconUrl: rootIconUrl })
    }

    function setDeviceDetails(device, rootIndex, rootIconUrl) {
        loader.setSource("DeviceDetails.qml", { device: device, rootIndex: rootIndex, rootIconUrl: rootIconUrl })
    }

    Component.onCompleted: {
        setRootDeviceListView()
    }
}
