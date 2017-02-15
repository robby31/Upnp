import QtQuick 2.0
import MyComponents 1.0

Column {
    id: item
    anchors.fill: parent
    anchors.margins: 10
    spacing: 10

    property alias text: message.text

    MyButton {
        sourceComponent: Text { id: text; text: "< Back" }
        onButtonClicked: setMessageListView()
    }

    Text {
        id: message
    }
}
