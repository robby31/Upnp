import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

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

    RowLayout {
        id: row
        anchors { left: parent.left; right: parent.right; margins: 10 }
        height: actionName.height+10
        spacing: 10

        Label {
            id: actionName
            Layout.alignment: Qt.AlignVCenter
            text: modelData
            Layout.fillWidth: true
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
            delegate.ListView.view.runAction(index)
        }
    }
}

