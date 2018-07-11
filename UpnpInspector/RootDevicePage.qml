import QtQuick 2.9
import QtQuick.Controls 2.2
import MyComponents 1.0

Page {
    id: page
    width: 200
    height: 100

    property var upnpControlPoint

    StackView {
         id: stack
         anchors.fill: parent

         Component.onCompleted: stack.push("RootDeviceListView.qml", { model: upnpControlPoint.remoteRootDevices })
     }

    function goBack() {
        stack.pop()
    }

    function setRootDeviceDetails(index, iconurl) {
        stack.push("RootDeviceDetails.qml", { device: upnpControlPoint.remoteRootDevices.at(index), iconurl: iconurl})
    }

    function setServiceDetails(service, rootIconUrl) {
        if (service.serviceType.startsWith("urn:schemas-upnp-org:service:ConnectionManager:"))
            stack.push("ServiceConnectionManager.qml", { service: service, rootIconUrl: rootIconUrl })
        else
            stack.push("ServiceDetails.qml", { service: service, rootIconUrl: rootIconUrl })
    }

    function setDeviceDetails(device, rootIconUrl) {
        stack.push("DeviceDetails.qml", { device: device, rootIconUrl: rootIconUrl })
    }
}
