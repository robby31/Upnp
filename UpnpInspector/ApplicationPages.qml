import QtQuick 2.9

Item {
    id: pages

    property var messageModel
    property var upnpControlPoint

    function toggleMenu() {
        if (mainWindow.state==="MESSAGES")
            messagePage.toggleMenu()
        else if (mainWindow.state==="DEVICES")
            rootDevicePage.toggleMenu()
    }

    MessagePage {
        id: messagePage
        anchors.fill: parent
        visible: mainWindow.state === "MESSAGES"
        messageModel: pages.messageModel
    }

    RootDevicePage {
        id: rootDevicePage
        anchors.fill: parent
        visible: mainWindow.state === "DEVICES"
        upnpControlPoint: pages.upnpControlPoint
    }
}
