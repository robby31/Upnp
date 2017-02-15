import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
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
