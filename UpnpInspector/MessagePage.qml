import QtQuick 2.9
import QtQuick.Controls 2.2
import MyComponents 1.0

Page {
    id: page
    width: 200
    height: 100

    property var messageModel

    StackView {
         id: stack
         anchors.fill: parent

         Component.onCompleted: setMessageListView()
     }

    function goBack() {
        stack.pop()
    }

    function setMessageDetails(message) {
        stack.push("MessageDetails.qml", { text: message })
    }

    function setMessageListView() {
        stack.push("MessageListView.qml", { model: messageModel })
    }
}
