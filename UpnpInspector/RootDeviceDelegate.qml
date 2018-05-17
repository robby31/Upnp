import QtQuick 2.9
import QtQuick.Controls 2.2
import MyComponents 1.0

ListViewDelegate {
    id: delegate
    width: parent ? parent.width : 0
    height: 50

    property color color: available ? "black" : "red"

    onDoubleClicked: delegate.ListView.view.selectDetails(index, iconurl)

    contentItem: Item {

        Row {
            id: row
            spacing: 10
            anchors.fill: parent

            Image {
                anchors { verticalCenter: parent.verticalCenter }
                width: 30
                height: width
                sourceSize.width: width
                sourceSize.height: width
                fillMode: Image.PreserveAspectFit
                source: iconurl
            }

            Label {
                id: hostLabel
                anchors { verticalCenter: parent.verticalCenter }
                text: host
                width: 150
                elide: Text.ElideRight
                color: delegate.color
            }

            Label {
                anchors { verticalCenter: parent.verticalCenter }
                text: friendlyname
                width: 400
                elide: Text.ElideRight
                color: delegate.color
            }

            Label {
                anchors { verticalCenter: parent.verticalCenter }
                text: "UPNP %1".arg(version)
                width: 100
                elide: Text.ElideRight
                color: delegate.color
            }

            Label {
                anchors { verticalCenter: parent.verticalCenter }
                text: devicetype
                width: 400
                elide: Text.ElideRight
                color: delegate.color
            }
        }
    }
}
