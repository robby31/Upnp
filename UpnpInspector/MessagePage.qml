import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import MyComponents 1.0

Page {
    id: page
    width: 200
    height: 100

    property var messageModel

    Loader {
        id: loader

        anchors.fill: parent
    }

    function setMessageDetails(message) {
        loader.setSource("MessageDetails.qml", {text: message})
    }

    function setMessageListView() {
        loader.setSource("MessageListView.qml", { model: messageModel })
    }

    Component.onCompleted: {
        setMessageListView()
    }
}
