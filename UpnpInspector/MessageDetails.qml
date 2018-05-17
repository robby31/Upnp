import QtQuick 2.9
import MyComponents 1.0

Item {
    id: item

    property alias text: message.text

    Column {
        anchors { fill: parent; margins: 10 }
        spacing: 10

        MyButton {
            sourceComponent: Text { id: text; text: "< Back" }
            onButtonClicked: goBack()
        }

        Text {
            id: message
        }
    }
}
