import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

Item {
    id: delegate
    width: parent.width
    height: row.height

    Rectangle {
        id: hover
        anchors.fill: parent
        color: theme.hoverColor
        visible: mouseArea.containsMouse
    }

    Rectangle {
        id: highlight
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: theme.highlightGradientStart }
            GradientStop { position: 1.0; color: theme.highlightGradientEnd }
        }
        visible: mouseArea.pressed
    }

    Row {
        id: row
        anchors { left: parent.left; right: parent.right; margins: 10 }
        height: hostLabel.height+10
        spacing: 10

        Label {
            anchors { verticalCenter: parent.verticalCenter }
            text: date
            width: 100
            elide: Text.ElideRight
        }

        Label {
            anchors { verticalCenter: parent.verticalCenter }
            text: type
            width: 150
            elide: Text.ElideRight
        }

        Label {
            id: hostLabel
            anchors { verticalCenter: parent.verticalCenter }
            text: host
            width: 150
            elide: Text.ElideRight
        }

        Label {
            anchors { verticalCenter: parent.verticalCenter }
            text: port
            width: 50
            elide: Text.ElideRight
        }
    }

    Rectangle {
        id: separatorBottom
        width: parent.width
        height: 1
        anchors { left: parent.left; bottom: parent.bottom }
        color: theme.separatorColor
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            delegate.ListView.view.selectMessage(index, message)
        }
    }
}
