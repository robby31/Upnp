import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    id: delegate
    width: parent.width
    height: row.height

    property color color: available ? "black" : "red"

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

    RowLayout {
        id: row
        anchors { left: parent.left; right: parent.right; margins: 10 }
        height: hostLabel.height+10
        spacing: 10

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
            text: devicetype
            Layout.fillWidth: true
            elide: Text.ElideRight
            color: delegate.color
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
            delegate.ListView.view.selectDevice(index)
        }
    }
}
