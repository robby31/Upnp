import QtQuick 2.9
import MyComponents 1.0

MyApplication {
    id: mainWindow
    title: qsTr("UPNP Inspector")
    width: 640
    height: 480

    modelButtons: mybuttons

    ListModel {
        id: mybuttons
        ListElement {
            title: "Messages"
            state: "MESSAGES"
            icon: ""
        }

        ListElement {
            title: "Devices"
            state: "DEVICES"
            icon: ""
        }
    }

    Component.onCompleted: {
        setPage(Qt.resolvedUrl("ApplicationPages.qml"),
                { messageModel: _app.messageModel,
                  upnpControlPoint: _app.upnpControlPoint })
    }
}
